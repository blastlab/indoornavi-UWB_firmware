#include "mac.h"
#include "../settings.h"

// global mac instance
mac_instance_t mac;

mac_buf_t *mac_buf_get_oldest_to_tx();
void _mac_buffer_reset(mac_buf_t *buf);
void mac_try_transmit_frame_in_slot(int64_t time);

void mac_tx_cb(const dwt_cb_data_t *data);
void mac_rx_cb(const dwt_cb_data_t *data);
void mac_rx_to_cb(const dwt_cb_data_t *data);
void mac_rx_er_cb(const dwt_cb_data_t *data);

void mac_init()
{
  transceiver_init(settings.mac.pan, settings.mac.addr);
  transceiver_set_cb(mac_tx_cb, mac_rx_cb, mac_rx_to_cb, mac_rx_er_cb);
}

void mac_tx_cb(const dwt_cb_data_t *data)
{
  const mac_buf_t *buf = mac.buf_under_tx;
  int64_t tx_ts = transceiver_get_tx_timestamp();

  port_led_on(LED_STAT);

  // zero means that no next frame has been send as
  // a ranging frame in a called callback
  int ret = 0;

  if (buf->isRangingFrame)
  {
    // try ranging callback
  }

  // try send next data frame
  if (ret == 0)
  {
    mac_try_transmit_frame_in_slot(tx_ts);
  }
}

void mac_rx_cb(const dwt_cb_data_t *data)
{
  int ret = 0;

  port_led_on(LED_STAT);
  mac.last_rx_ts = port_tick_hr();
  mac_buf_t *buf = mac_buffer();
  if (buf != 0)
  {
    transceiver_read(buf->buf, data->datalength);

    if (data->rx_flags & DWT_CB_DATA_RX_FLAG_RNG)
    {
      // process ranging
    }
    else
    {
      // parse data
    }
  }
  else
  {
    LOG_ERR("No buff for rx_cb");
  }
}

void mac_rx_to_cb(const dwt_cb_data_t *data)
{
  // ranging isr
  port_led_on(LED_ERR);
}

void mac_rx_er_cb(const dwt_cb_data_t *data)
{
  // mayby some log?
  port_led_on(LED_ERR);
}

// get time from start of super frame in mac_get_port_sync_time units
int64_t mac_to_slots_time(int64_t sync_time_raw)
{
  int super_time =
      (sync_time_raw - mac.sync_offset) % settings.mac.slots_sum_time;
  return super_time;
}

void mac_your_slot_isr()
{
  int64_t time = transceiver_get_time();
  mac_try_transmit_frame_in_slot(time);
}

// calc slot time and send try send packet if it is yours time
void mac_try_transmit_frame_in_slot(int64_t time)
{
  // calc time from begining of yours slot
  int64_t slot_time = mac_to_slots_time(time);
  slot_time -= settings.mac.slot_number * settings.mac.slot_time;
  slot_time -= settings.mac.slot_guard_time;
  if (slot_time > settings.mac.slot_time)
  {
    return;
  }

  mac_buf_t *buf = mac_buf_get_oldest_to_tx();
  if (buf == 0)
  {
    return;
  }
  int len = mac_buf_len(buf);
  unsigned int tx_est_time = transceiver_estimate_tx_time_us(len);
  if (slot_time + tx_est_time > settings.mac.slot_time)
  {
    return;
  }

  // when you have enouth time to send next message, then do it
  mac.buf_under_tx = buf;
  int ret;
  if (buf->isRangingFrame)
  {
    const uint8_t flags = DWT_START_RX_IMMEDIATE | DWT_RESPONSE_EXPECTED;
    ret = transceiver_send_ranging(buf->buf, len, flags);
  }
  else
  {
    ret = transceiver_send(buf->buf, len);
  }
  // check result
  if (ret == 0)
  {
    buf->last_update_time = mac_port_buff_time();
    buf->state = buf->state == WAIT_FOR_TX_ACK ? WAIT_FOR_ACK : FREE;
  }
  else
  {
    buf->last_update_time = mac_port_buff_time();
    ++buf->retransmit_fail_cnt;
    if (buf->retransmit_fail_cnt > settings.mac.max_frame_fail_cnt)
    {
      buf->state = FREE;
    }
    // try send next frame
    mac_try_transmit_frame_in_slot(transceiver_get_time());
  }
}

// call this function when ACK arrive
void mac_ack_frame_isr(uint8_t seq_num)
{
  for (int i = 0; i < MAC_BUF_CNT; ++i)
  {
    if (mac.buf[i].state == WAIT_FOR_ACK)
    {
      mac.buf[i].state = FREE;
      break;
    }
  }
}

void _mac_buffer_reset(mac_buf_t *buf)
{
  buf->dPtr = buf->buf;
  buf->state = BUSY;
  buf->retransmit_fail_cnt = 0;
  buf->last_update_time = mac_port_buff_time();
}

// get pointer to the oldest buffer with WAIT_FOR_TX or WAIT_FOR_TX_ACK
// when there is no buffer to tx then return 0
mac_buf_t *mac_buf_get_oldest_to_tx()
{
  int oldest_index = MAC_BUF_CNT;
  int current_time = mac_port_buff_time();
  int oldest_time = current_time - mac.buf[0].last_update_time;

  for (int i = 0; i < MAC_BUF_CNT; ++i)
  {
    mac_buf_state state = mac.buf[i].state;
    if (state == WAIT_FOR_TX || state == WAIT_FOR_TX_ACK)
    {
      int buff_age = current_time - mac.buf[i].last_update_time;
      if (buff_age > oldest_time)
      {
        oldest_index = i;
        oldest_time = buff_age;
      }
    }
  }

  return oldest_index < MAC_BUF_CNT ? &mac.buf[oldest_index] : 0;
}

// reserve buffer
mac_buf_t *mac_buffer()
{
  int oldest_index = 0;
  int current_time = mac_port_buff_time();
  int oldest_time = current_time - mac.buf[0].last_update_time;
  mac_buf_t *buf = &mac.buf[mac.buf_get_ind];

  // next buffer buffer is FREE so return them
  if (buf->state == FREE)
  {
    mac.buf_get_ind = (mac.buf_get_ind + 1) % MAC_BUF_CNT;
    _mac_buffer_reset(buf);
    return buf;
  }
  printf("search\n");
  // else search for empty buffer
  for (int i = 0; i < MAC_BUF_CNT; ++i)
  {
    printf("i");
    int buff_age = current_time - mac.buf[i].last_update_time;
    if (mac.buf[i].state == FREE)
    {
      buf = &mac.buf[i];
      _mac_buffer_reset(buf);
      return buf;
    }
    else if (buff_age > oldest_time)
    {
      oldest_index = i;
      oldest_time = buff_age;
    }
  }

  // // else get the oldest one
  // MAC_LOG_ERR("mac_buffer() overload");
  // buf = mac.buf[oldest_index];
  // _mac_buffer_reset(buf);
  // return buf;

  // else return null
  return 0;
}

void mac_fill_frame_to(mac_buf_t *buf, dev_addr_t target)
{
  MAC_ASSERT(buf != 0);
  buf->frame.src = settings.mac.addr;
  buf->frame.dst = target;
  buf->dPtr = &buf->frame.data[0];
}

mac_buf_t *mac_buffer_prepare(dev_addr_t target, bool can_append)
{
  mac_buf_t *buf;
  // search buffer to this target
  if (can_append)
  {
    for (int i = 0; i < MAC_BUF_CNT; ++i)
    {
      buf = &mac.buf[i];
      if (buf->state == WAIT_FOR_TX || buf->state == WAIT_FOR_TX_ACK)
      {
        // do not use mac_fill_frame_to, becouse the buff is already partially
        // filled
        buf->state = BUSY;
        return buf;
      }
    }
  }

  // else get free buffer
  buf = mac_buffer();
  if (buf != 0)
  {
    mac_fill_frame_to(buf, target);
  }
  return buf;
}

int mac_buf_len(const mac_buf_t *buf)
{
  MAC_ASSERT(buf != 0);
  MAC_ASSERT(buf->dPtr >= buf->buf);
  return (int)(buf->dPtr - (uint8_t *)(buf));
}

// free buffer
void mac_free(mac_buf_t *buff)
{
  MAC_ASSERT((uint8_t *)buff >= (uint8_t *)mac.buf);
  int i = ((char *)buff - (char *)mac.buf) / sizeof(*buff);
  MAC_ASSERT(i < MAC_BUF_CNT);
  mac.buf[i].state = FREE;
}

// add frame to the transmit queue
void mac_send(mac_buf_t *buf, bool ack_require)
{
  MAC_ASSERT(buf != 0);
  if (ack_require)
  {
    buf->state = WAIT_FOR_TX_ACK;
  }
  else
  {
    buf->state = WAIT_FOR_TX;
  }
  buf->last_update_time = mac_port_buff_time();
}

// add frame to the transmit queue
int mac_send_ranging_resp(mac_buf_t *buf, uint8_t transceiver_flags)
{
  MAC_ASSERT(buf != 0);
  int len = mac_buf_len(buf);
  buf->isRangingFrame = true;
  buf->frame.dst = buf->frame.src;
  buf->frame.src = settings.mac.addr;
  int ret = transceiver_send_ranging(buf->buf, len, transceiver_flags);
  buf->state = FREE;
  return ret;
}

unsigned int mac_us_from_rx()
{
  float diff = port_tick_hr() - mac.last_rx_ts;
  diff *= 1e6;
  diff /= port_freq_hr();
  return (unsigned int)diff;
}

unsigned char mac_read8(mac_buf_t *frame)
{
  MAC_ASSERT(frame != 0);
  MAC_ASSERT(frame->dPtr >= (uint8_t *)frame);
  return *frame->dPtr++;
}

void mac_write8(mac_buf_t *frame, unsigned char value)
{
  MAC_ASSERT(frame != 0);
  MAC_ASSERT(frame->dPtr >= (uint8_t *)frame);
  *frame->dPtr = value;
  ++frame->dPtr;
}

void mac_read(mac_buf_t *frame, void *destination, unsigned int len)
{
  MAC_ASSERT(frame != 0);
  MAC_ASSERT(frame->dPtr >= (uint8_t *)frame);
  MAC_ASSERT(destination != 0);
  MAC_ASSERT(0 <= len && len < MAC_BUF_LEN);
  uint8_t *dst = (uint8_t *)destination;
  while (len > 0)
  {
    *dst = *frame->dPtr;
    ++dst;
    ++frame->dPtr;
    --len;
  }
}

void mac_write(mac_buf_t *frame, const void *source, unsigned int len)
{
  MAC_ASSERT(frame != 0);
  MAC_ASSERT(frame->dPtr >= (uint8_t *)frame);
  MAC_ASSERT(source != 0);
  MAC_ASSERT(0 <= len && len < MAC_BUF_LEN);
  const uint8_t *src = (uint8_t *)source;
  while (len > 0)
  {
    *frame->dPtr = *src;
    ++frame->dPtr;
    ++src;
    --len;
  }
}
