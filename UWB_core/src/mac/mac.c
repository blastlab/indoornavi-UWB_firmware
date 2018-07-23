#include "mac.h"
#include "../settings.h"
#include "carry.h"
#include "sync.h"
#include "toa_routine.h"

// global mac instance
mac_instance_t mac;

static mac_buf_t *_MAC_BufGetOldestToTx();
static void _MAC_BufferReset(mac_buf_t *buf);
int MAC_TryTransmitFrameInSlot(int64_t glob_time);

static void MAC_TxCb(const dwt_cb_data_t *data);
static void MAC_RxCb(const dwt_cb_data_t *data);
static void MAC_RxToCb(const dwt_cb_data_t *data);
static void MAC_RxErrCb(const dwt_cb_data_t *data);

void MAC_Init() {
  // init transceiver
  TRANSCEIVER_Init();

  // get local address
  if (settings.mac.addr == ADDR_BROADCAST) {
    settings.mac.addr = (dev_addr_t)dwt_getpartid();
  }

  // apply anchor address flag
  if (settings.mac.role == RTLS_TAG) {
    settings.mac.addr &= ~ADDR_ANCHOR_FLAG;
  } else {
    settings.mac.addr |= ADDR_ANCHOR_FLAG;
  }

  // set address and irq callbacks in transceiver
  if (settings.mac.role == RTLS_LISTENER) {
    TRANSCEIVER_SetCb(0, listener_isr, MAC_RxToCb, MAC_RxErrCb);
  } else {
    TRANSCEIVER_SetCb(MAC_TxCb, MAC_RxCb, MAC_RxToCb, MAC_RxErrCb);
  }

  // initialize synchronization and ranging engine
  SYNC_Init();
  TOA_InitDly();

  // slot timers
  PORT_SetSlotTimerPeriodUs(settings.mac.slots_sum_time_us);

  // turn on receiver after full low level initialization
  // especially after connecting callbacks
  TRANSCEIVER_DefaultRx();

  // prevent beacon sending at startup
  MAC_BeaconTimerReset();
}

static void MAC_TxCb(const dwt_cb_data_t *data) {
  int64_t tx_ts = TRANSCEIVER_GetTxTimestamp();

  PORT_LedOn(LED_STAT);

  // zero means that no next frame has been send as
  // a ranging frame in a called callback
  int ret = 0;

  if (mac.frame_under_tx_is_ranging) {
    // try ranging callback
    ret = SYNC_TxCb(tx_ts);
    if(ret != 0) {
      ret = TOA_TxCb(tx_ts);
    }
    // reset ranging settings when SYNC release transceiver
    if (ret == 0) {
      mac.frame_under_tx_is_ranging = false;
      dwt_forcetrxoff();
    }
  }

  // try send next data frame
  if (ret == 0) {
    ret = MAC_TryTransmitFrameInSlot(SYNC_GlobTime(tx_ts));
  }

  // or turn on default rx mode
  if (ret == 0) {
    // when ret is 0 then no frame has to be transmitted
    // and it wasn't ranging frame, so turn on receiver after tx
    if (ret == 0) {
      //
      dwt_forcetrxoff();
      TRANSCEIVER_DefaultRx();
    }
  }
}

static void MAC_RxCb(const dwt_cb_data_t *data) {
  PORT_LedOn(LED_STAT);
  mac.last_rx_ts = PORT_TickHr();
  mac_buf_t *buf = MAC_Buffer();
  prot_packet_info_t info;
  memset(&info, 0, sizeof(info));
  bool broadcast, unicast;

  if (buf != 0) {
    TRANSCEIVER_Read(buf->buf, data->datalength);
    buf->rx_len = data->datalength - 2; // ommit CRC
    info.direct_src = buf->frame.src;
    broadcast = buf->frame.dst == ADDR_BROADCAST;
    unicast = buf->frame.dst == settings.mac.addr;

    if(unicast) {
    	MAC_BeaconTimerReset();
    }

    if (broadcast || unicast) {
      int type = buf->frame.control[0] & FR_CR_TYPE_MASK;
      if (type == FR_CR_MAC) {
        // int ret = SYNC_UpdateNeighbour()
        int ret = SYNC_RxCb(buf->frame.data, &info);
        if(ret == 0){
          ret = TOA_RxCb(buf->frame.data, &info);
        }
        if(ret == 0) {
          LOG_WRN("Unsupported MAC frame %X", buf->frame.data[0]);
        }
      } else if (type == FR_CR_DATA) {
        TRANSCEIVER_DefaultRx();
        CARRY_ParseMessage(buf);
      } else if(type == FR_CR_BEACON){
        prot_packet_info_t info;
        memset(&info, 0, sizeof(info));
        info.direct_src = buf->frame.src;
        BIN_ParseSingle(buf->dPtr, &info);
      } else if(type == FR_CR_ACK) {
        LOG_WRN("ACK frame is not supported");
      } else {
        LOG_ERR("This kind of frame is not supported: %x",
        buf->frame.control[0]);
        TRANSCEIVER_DefaultRx();
      }
    } else {
      // frame not for you
      TRANSCEIVER_DefaultRx();
    }
    MAC_Free(buf);
  } else {
    LOG_ERR("No buff for rx_cb");
    TRANSCEIVER_DefaultRx();
  }
}

// timeout error -> check ranging, maybe ACK or default RX
static void MAC_RxToCb(const dwt_cb_data_t *data) {
  // ranging isr
  PORT_LedOn(LED_ERR);
  int ret = SYNC_RxToCb();
  if(ret != 0) {
    ret = TOA_RxToCb();
  }
  if (ret == 0) {
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
    LOG_DBG("MAC_RxToCb");
  }
}

// error during receiving frame
static void MAC_RxErrCb(const dwt_cb_data_t *data) {
  // mayby some log?
  PORT_LedOn(LED_ERR);
  LOG_ERR("Rx error status:%X", data->status);
  TRANSCEIVER_DefaultRx();
}


// return ms from last BeconTimerReset or received unicast message
unsigned int MAC_BeaconTimerGetMs()
{
	decaIrqStatus_t en = decamutexon();
	unsigned int ret = PORT_TickMs() - mac.beacon_timer_timestamp;
	decamutexoff(en);
	return ret;
}

// reset beacon timer after sending a beacon message
void MAC_BeaconTimerReset()
{
	mac.beacon_timer_timestamp = PORT_TickMs();
}

// get time from start of super frame in mac_get_port_sync_time units
int MAC_ToSlotsTimeUs(int64_t glob_time) {
	int64_t slots_period_dtu = (int64_t)settings.mac.slots_sum_time_us / (DWT_TIME_UNITS * 1e6f); // multiple in two lines (conversion 32-64B)
	int slot_time_us = (glob_time % slots_period_dtu) * (DWT_TIME_UNITS * 1e6f);
	return slot_time_us;
}

// update MAC slot timer to be in time with global time
void MAC_UpdateSlotTimer(int32_t loc_slot_time_us, int64_t local_time) {
	extern sync_instance_t sync;
	int64_t glob_time = SYNC_GlobTime(local_time);
	int slot_time_us = MAC_ToSlotsTimeUs(glob_time);
	int time_to_your_slot_us = settings.mac.slot_time_us * mac.slot_number - slot_time_us;

	if (time_to_your_slot_us <= 0) {
	  time_to_your_slot_us += settings.mac.slots_sum_time_us;
	}

	PORT_SlotTimerSetUsOffset(time_to_your_slot_us - loc_slot_time_us);
	MAC_TRACE("SYNC %7d %7d %4d", (int)time_to_your_slot_us, PORT_SlotTimerTickUs(), (int)sync.neighbour[0].drift[0]);
}

// Function called from slot timer interrupt.
void MAC_YourSlotIsr() {
  CRITICAL(
    int64_t local_time = TRANSCEIVER_GetTime();
    uint32_t slot_time = PORT_SlotTimerTickUs();
    mac.slot_time_offset = SYNC_GlobTime(local_time);
    MAC_UpdateSlotTimer(slot_time, local_time);
  )
  decaIrqStatus_t en = decamutexon();
  MAC_TryTransmitFrameInSlot(mac.slot_time_offset);
  decamutexoff(en);
}

// private function, called when buf should be send now as a frame in slot
static void _MAC_TransmitFrameInSlot(mac_buf_t *buf, int len) {
  int ret;
  // POLL is send through queue and need DWT_RESPONSE_EXPECTED flag
  if (buf->isRangingFrame) {
    const uint8_t flags = DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED;
    dwt_forcetrxoff();
    ret = MAC_SendRanging(buf, flags);
  } else {
    ret = TRANSCEIVER_Send(buf->buf, len);
  }
  buf->last_update_time = mac_port_buff_time();
  // check result
  if (ret == 0) {
    buf->state = (buf->state == WAIT_FOR_TX_ACK) ? WAIT_FOR_ACK : FREE;
    // wait for tx callback
  } else {
    ++buf->retransmit_fail_cnt;
    if (buf->retransmit_fail_cnt > settings.mac.max_frame_fail_cnt) {
      buf->state = FREE;
    }
    // try send next frame after tx fail
    int64_t glob_time = SYNC_GlobTime(TRANSCEIVER_GetTime());
    MAC_TryTransmitFrameInSlot(glob_time);
    LOG_WRN("Tx err");
  }
}

// calc slot time and send try send packet if it is yours time
int MAC_TryTransmitFrameInSlot(int64_t glob_time) {
  // calc time from begining of yours slot
  int64_t slot_time = MAC_ToSlotsTimeUs(glob_time);
  if (settings.mac.slots_sum_time_us < slot_time || slot_time < 0) {
    return 0;
  }

  mac_buf_t *buf = _MAC_BufGetOldestToTx();
  if (buf == 0) {
    return 0;
  }
  int len = MAC_BufLen(buf);
  uint32_t tx_est_time = TRANSCEIVER_EstimateTxTimeUs(len);
  uint32_t end_us = settings.mac.slot_time_us * (mac.slot_number + 1);
  end_us -= settings.mac.slot_guard_time_us;
  if(tx_est_time > settings.mac.slot_time_us - settings.mac.slot_guard_time_us) {
    LOG_WRN("Frame with size %d can't be send within %dus slot", len, settings.mac.slot_time_us);
    MAC_Free(buf);
  }
  // when it is too late to send this packet
  if (end_us < slot_time + tx_est_time) {
    return 0;
  }

  if (SYNC_GlobTime(glob_time) == mac.slot_time_offset) {
    dwt_rxreset();
  }
  // when you have enouth time to send next message, then do it
  _MAC_TransmitFrameInSlot(buf, len);
  return 1;
}

// call this function when ACK arrive
void MAC_AckFrameIsr(uint8_t seq_num) {
  for (int i = 0; i < MAC_BUF_CNT; ++i) {
    if (mac.buf[i].state == WAIT_FOR_ACK) {
      mac.buf[i].state = FREE;
      break;
    }
  }
}

static void _MAC_BufferReset(mac_buf_t *buf) {
  buf->state = BUSY;
  buf->dPtr = buf->buf;
  buf->retransmit_fail_cnt = 0;
  buf->last_update_time = mac_port_buff_time();
  buf->isRangingFrame = false;
  buf->rx_len = 0;
}

// get pointer to the oldest buffer with WAIT_FOR_TX or WAIT_FOR_TX_ACK
// when there is no buffer to tx then return 0
static mac_buf_t *_MAC_BufGetOldestToTx() {
  volatile int oldest_index = MAC_BUF_CNT;
  int current_time = mac_port_buff_time();
  int oldest_time = -1;
  volatile int buff_age;
  volatile mac_buf_state state;

  for (int i = 0; i < MAC_BUF_CNT; ++i) {
    state = mac.buf[i].state;
    if (state == WAIT_FOR_TX || state == WAIT_FOR_TX_ACK) {
      buff_age = current_time - mac.buf[i].last_update_time;
      if (buff_age > oldest_time) {
        oldest_index = i;
        oldest_time = buff_age;
      }
    }
  }

  return oldest_index < MAC_BUF_CNT ? &mac.buf[oldest_index] : 0;
}

// reserve buffer
mac_buf_t *MAC_Buffer() {
  mac_buf_t *buf = &mac.buf[mac.buf_get_ind];

  // next buffer buffer is FREE so return them
  if (buf->state == FREE) {
    mac.buf_get_ind = (mac.buf_get_ind + 1) % MAC_BUF_CNT;
    _MAC_BufferReset(buf);
    return buf;
  }

  // else search for empty buffer
  for (int i = 0; i < MAC_BUF_CNT; ++i) {
    if (mac.buf[i].state == FREE) {
      buf = &mac.buf[i];
      _MAC_BufferReset(buf);
      return buf;
    }
  }

  // else search for old buffer
  mac_buff_time_t now = mac_port_buff_time();
  for (int i = 0; i < MAC_BUF_CNT; ++i) {
    mac_buff_time_t age = now - buf[i].last_update_time;
    if (age > settings.mac.max_buf_inactive_time) {
      _MAC_BufferReset(&buf[i]);
      return buf;
    }
  }
  // else return null
  return 0;
}

void MAC_FillFrameTo(mac_buf_t *buf, dev_addr_t target) {
  MAC_ASSERT(buf != 0);
  buf->frame.control[0] = FR_CR_DATA;
  buf->frame.control[1] = FR_CRH_DA_SHORT | FR_CRH_SA_SHORT | FR_CRH_FVER0;
  buf->frame.seq_num = ++mac.seq_num;
  buf->frame.pan = settings.mac.pan;
  buf->frame.dst = target;
  buf->frame.src = settings.mac.addr;
  buf->dPtr = &buf->frame.data[0];
}

void MAC_SetFrameType(mac_buf_t *buf, uint8_t FR_CR_type) {
  MAC_ASSERT(buf != 0);
  MAC_ASSERT(FR_CR_type == FR_CR_ACK || FR_CR_type == FR_CR_BEACON || FR_CR_type == FR_CR_DATA ||
  		FR_CR_type == FR_CR_MAC);
  buf->frame.control[0] =
      (buf->frame.control[0] & ~FR_CR_TYPE_MASK) | FR_CR_MAC;
}

mac_buf_t *MAC_BufferPrepare(dev_addr_t target, bool can_append) {
  mac_buf_t *buf;
  // search buffer to this target
  if (can_append) {
    for (int i = 0; i < MAC_BUF_CNT; ++i) {
      buf = &mac.buf[i];
      if (buf->state == WAIT_FOR_TX || buf->state == WAIT_FOR_TX_ACK) {
        // do not use mac_fill_frame_to, becouse the buff is already partially
        // filled
        FC_CARRY_s *carry = (FC_CARRY_s *)&buf->frame.data[0];
        if (carry->hops[0] == target) {
          buf->state = BUSY;
          return buf;
        }
      }
    }
  }

  // else get free buffer
  buf = MAC_Buffer();
  if (buf != 0) {
    MAC_FillFrameTo(buf, target);
  }
  return buf;
}

int MAC_BufLen(const mac_buf_t *buf) {
  MAC_ASSERT(buf != 0);
  MAC_ASSERT(buf->dPtr >= buf->buf);
  return (int)(buf->dPtr - (uint8_t *)(buf));
}

// free buffer
void MAC_Free(mac_buf_t *buff) {
  MAC_ASSERT((uint8_t *)buff >= (uint8_t *)mac.buf);
  int i = ((char *)buff - (char *)mac.buf) / sizeof(*buff);
  MAC_ASSERT(i < MAC_BUF_CNT);
  mac.buf[i].state = FREE;
  // where it last allocated buffer, then decrement buf_get_ind
  if (mac.buf_get_ind == (i + 1) % MAC_BUF_CNT) {
    mac.buf_get_ind = i;
  }
}

// add frame to the transmit queue
void MAC_Send(mac_buf_t *buf, bool ack_require) {
  MAC_ASSERT(buf != 0);
  MAC_ASSERT((buf->frame.dst == ADDR_BROADCAST && ack_require == true) ==
             false);
  if (ack_require) {
    buf->state = WAIT_FOR_TX_ACK;
  } else {
    buf->state = WAIT_FOR_TX;
  }
  buf->last_update_time = mac_port_buff_time();
}

// add frame to the transmit queue
int MAC_SendRanging(mac_buf_t *buf, uint8_t transceiver_flags) {
  MAC_ASSERT(buf != 0);
  int len = MAC_BufLen(buf);
  mac.frame_under_tx_is_ranging = buf->isRangingFrame = true;
  buf->frame.src = settings.mac.addr;
  MAC_SetFrameType(buf, FR_CR_MAC);
  int ret = TRANSCEIVER_SendRanging(buf->buf, len, transceiver_flags);
  buf->state = FREE;
  return ret;
}

unsigned int MAC_UsFromRx() {
  uint32_t diff = PORT_TickHr() - mac.last_rx_ts;
  return PORT_TickHrToUs(diff);
}

unsigned char MAC_Read8(mac_buf_t *frame) {
  MAC_ASSERT(frame != 0);
  MAC_ASSERT(frame->dPtr >= (uint8_t *)frame);
  return *frame->dPtr++;
}

void MAC_Write8(mac_buf_t *frame, unsigned char value) {
  MAC_ASSERT(frame != 0);
  MAC_ASSERT(frame->dPtr >= (uint8_t *)frame);
  *frame->dPtr = value;
  ++frame->dPtr;
}

void MAC_Read(mac_buf_t *frame, void *destination, unsigned int len) {
  MAC_ASSERT(frame != 0);
  MAC_ASSERT(frame->dPtr >= (uint8_t *)frame);
  MAC_ASSERT(destination != 0);
  MAC_ASSERT(0 <= len && len < MAC_BUF_LEN);
  uint8_t *dst = (uint8_t *)destination;
  while (len > 0) {
    *dst = *frame->dPtr;
    ++dst;
    ++frame->dPtr;
    --len;
  }
}

void MAC_Write(mac_buf_t *frame, const void *source, unsigned int len) {
  MAC_ASSERT(frame != 0);
  MAC_ASSERT(frame->dPtr >= (uint8_t *)frame);
  MAC_ASSERT(source != 0);
  MAC_ASSERT(0 <= len && len < MAC_BUF_LEN);
  const uint8_t *src = (uint8_t *)source;
  while (len > 0) {
    *frame->dPtr = *src;
    ++frame->dPtr;
    ++src;
    --len;
  }
}
