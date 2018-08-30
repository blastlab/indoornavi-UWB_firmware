/**
 * @brief Time Of Arrival routines
 *
 * @file toa_routine.c
 * @author Karol Trzcinski
 * @date 2018-07-20
 */

#include "toa_routine.h"
#include "mac.h"
#include "carry.h"

extern toa_instance_t toa;
extern mac_instance_t mac;

const char toa_bad_len_msg[] = "%s bad len %d!=%d";
#define PROT_CHECK_LEN(FC, len, expected)               \
  do {                                                  \
    if ((len) < (expected)) {                           \
      LOG_ERR(toa_bad_len_msg, #FC, (len), (expected)); \
      return -1;                                        \
    }                                                   \
  } while (0)

void TOA_InitDly() {
  const int rx_to_tx_delay = 30;  // us
  const float spi_speed = 20e6;   // Hz
  const int POLL_PROCESSING_TIME_US = 270;
  const int RESP_PROCESSING_TIME_US = 200;

  toa_settings_t* tset = &settings.mac.toa_dly;

  if(tset->guard_time_us != 0) {
	  return;
  }

  tset->guard_time_us = 50;  // us
  tset->rx_after_tx_offset_us = tset->guard_time_us;
  tset->rx_after_tx_offset_us += TRANSCEIVER_EstimateTxTimeUs(0);

  // add here 2 bytes for CRC
  int basePollLen = MAC_HEAD_LENGTH + sizeof(FC_SYNC_POLL_s);
  basePollLen += sizeof(dev_addr_t) * TOA_MAX_DEV_IN_POLL;
  const int respLen = MAC_HEAD_LENGTH + sizeof(FC_SYNC_RESP_s);
  const int baseFinLen = MAC_HEAD_LENGTH + sizeof(FC_SYNC_FIN_s);
  int len;

  for (int i = 0; i < TOA_MAX_DEV_IN_POLL; ++i) {
    len = basePollLen;
    tset->resp_dly_us[i] = POLL_PROCESSING_TIME_US;
    tset->resp_dly_us[i] += 1e6 * len / spi_speed;
    tset->resp_dly_us[i] += rx_to_tx_delay + tset->guard_time_us;
    tset->resp_dly_us[i] += TRANSCEIVER_EstimateTxTimeUs(len);
    tset->resp_dly_us[i] += i * (TRANSCEIVER_EstimateTxTimeUs(respLen) +
                                 RESP_PROCESSING_TIME_US + tset->guard_time_us);
  }

  len = baseFinLen + TOA_MAX_DEV_IN_POLL * 5;
  tset->fin_dly_us = RESP_PROCESSING_TIME_US;
  tset->fin_dly_us += 1e6 * len / spi_speed;
  tset->fin_dly_us += rx_to_tx_delay + tset->guard_time_us;
  tset->fin_dly_us += TRANSCEIVER_EstimateTxTimeUs(respLen);
}

int TOA_SendInit(dev_addr_t dst, const dev_addr_t anchors[], int anc_cnt) {
  // Init should be sent to dst device when it is anchor
  // otherwise to first anchor from array and tag address should be added
  // as a last anchor
  TOA_ASSERT(0 < anc_cnt && anc_cnt < TOA_MAX_DEV_IN_POLL);
  dev_addr_t tempTarget = dst & ADDR_ANCHOR_FLAG ? dst : anchors[0];		// TODO repair this, no measures with tags
  FC_CARRY_s* carry;
  mac_buf_t* buf = CARRY_PrepareBufTo(tempTarget, &carry);
  int anc_addr_len = anc_cnt * sizeof(dev_addr_t);
  if (buf == 0) {
    return -1;
  }

  FC_TOA_INIT_s packet = {
      .FC = FC_TOA_INIT,
      .len = sizeof(FC_TOA_INIT_s) + anc_cnt * sizeof(dev_addr_t),
      .num_poll_anchor = tempTarget == dst ? anc_cnt : anc_cnt + 1,
  };
  CARRY_Write(carry, buf, &packet, sizeof(FC_TOA_INIT_s));
  CARRY_Write(carry, buf, anchors, sizeof(dev_addr_t) * anc_cnt);
  if(tempTarget != dst) {
    CARRY_Write(carry, buf, &dst, sizeof(dev_addr_t));
  }

  toa.core.resp_ind = 0;
  toa.core.anc_in_poll_cnt = anc_cnt;
  toa.core.initiator = settings.mac.addr;
  memcpy(&toa.core.addr_tab[0], anchors, anc_addr_len);

  CARRY_Send(buf, false);
  return 0;
}

int TOA_SendPoll(const dev_addr_t anchors[], int anc_cnt) {
  TOA_ASSERT(0 < anc_cnt && anc_cnt < TOA_MAX_DEV_IN_POLL);
  mac_buf_t* buf = MAC_BufferPrepare(anc_cnt == 1 ? anchors[0] : ADDR_BROADCAST, false);
  int anc_addr_len = anc_cnt * sizeof(dev_addr_t);
  if (buf == 0) {
    return -1;
  }

  FC_TOA_POLL_s packet = {
      .FC = FC_TOA_POLL,
      .len = sizeof(FC_TOA_POLL_s) + anc_cnt * sizeof(dev_addr_t),
      .num_poll_anchor = anc_cnt,
  };
  MAC_Write(buf, &packet, sizeof(FC_TOA_POLL_s));
  MAC_Write(buf, anchors, sizeof(dev_addr_t) * anc_cnt);

  toa.core.resp_ind = 0;
  toa.core.anc_in_poll_cnt = anc_cnt;
  toa.core.initiator = settings.mac.addr;
  memcpy(&toa.core.addr_tab[0], anchors, anc_addr_len);

  // send this frame in your slot (through queue) but with ranging flage
  // mac module should set DWT_RESPONSE_EXPECTED flag before transmission
  buf->isRangingFrame = true;
  TOA_State(&toa.core, TOA_POLL_WAIT_TO_SEND);
  MAC_SetFrameType(buf, FR_CR_MAC);
  MAC_Send(buf, false);
  return 0;
}

int TOA_SendResp(int64_t PollDwRxTs) {
  toa_settings_t* tset = &settings.mac.toa_dly;
  int resp_dly_us = tset->resp_dly_us[toa.core.resp_ind];

  toa.core.TsRespTx = TOA_SetTxTime(PollDwRxTs, resp_dly_us);

  mac_buf_t* buf = MAC_BufferPrepare(toa.core.initiator, false);
  if (buf == 0) {
    TOA_State(&toa.core, TOA_IDLE);
    return 0;
  }
  FC_TOA_RESP_s packet = {
      .FC = FC_TOA_RESP,
      .len = sizeof(FC_TOA_RESP_s),
      .TsPollRx = toa.core.TsPollRx,
      .TsRespTx = toa.core.TsRespTx,
  };
  MAC_Write(buf, &packet, packet.len);

  int tx_to_rx_dly_us = tset->resp_dly_us[toa.core.anc_in_poll_cnt - 1];
  tx_to_rx_dly_us += tset->fin_dly_us - resp_dly_us;  // == delay to tx fin
  tx_to_rx_dly_us -= tset->rx_after_tx_offset_us;     // substract preamble time
  TOA_ASSERT(tx_to_rx_dly_us > 0);
  dwt_setrxaftertxdelay(tx_to_rx_dly_us);
  dwt_setrxtimeout(settings.mac.toa_dly.guard_time_us +
                   tset->rx_after_tx_offset_us);

  TOA_State(&toa.core, TOA_RESP_WAIT_TO_SEND);
  const int tx_flags = DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED;
  return MAC_SendRanging(buf, tx_flags);
}

int TOA_SendFinal() {
  toa_settings_t* tset = &settings.mac.toa_dly;
  int fin_dly_us = tset->resp_dly_us[toa.core.anc_in_poll_cnt - 1];
  fin_dly_us += tset->fin_dly_us;
  int64_t TsFinTx = TOA_SetTxTime(toa.core.TsPollTx, fin_dly_us);

  mac_buf_t* buf = MAC_BufferPrepare(toa.core.addr_tab[0], false);
  FC_TOA_FIN_s packet = {
      .FC = FC_TOA_FIN,
      .len = sizeof(FC_TOA_FIN_s),
      .slot_num = mac.slot_number,
      .TsPollTx = toa.core.TsPollTx,
  };
  // calc global time in place to keep 40B resolution
  TOA_Write40bValue(&packet.TsFinTxBuf[0], TsFinTx);

  int resp_rx_ts_len = sizeof(*packet.TsRespRx) * toa.core.anc_in_poll_cnt;
  packet.len += resp_rx_ts_len;
  memcpy(&packet.TsRespRx[0], &toa.core.TsRespRx[0], resp_rx_ts_len);
  MAC_Write(buf, &packet, packet.len);

  TOA_State(&toa.core, TOA_FIN_WAIT_TO_SEND);
  const int tx_flags = DWT_START_TX_DELAYED;
  return MAC_SendRanging(buf, tx_flags);
}

int TOA_SendRes(const measure_t* measure) {
  FC_TOA_RES_s packet = {
    .FC = FC_TOA_RES,
    .len = sizeof(FC_TOA_RES_s),
    .meas = *measure
  };
  FC_CARRY_s* carry;
  mac_buf_t* buf = CARRY_PrepareBufTo(CARRY_ADDR_SINK, &carry);
  if(buf != 0) {
    CARRY_Write(carry, buf, &packet, packet.len);
    CARRY_Send(buf, false);
  }
  return 0;
}

void FC_TOA_INIT_cb(const void* data, const prot_packet_info_t* info) {
  FC_TOA_INIT_s* packet = (FC_TOA_INIT_s*)data;
  TOA_ASSERT(packet->FC == FC_TOA_INIT);
  int expected_size = sizeof(*packet) + packet->num_poll_anchor * sizeof(dev_addr_t);
  if(packet->len != expected_size) {
	  LOG_ERR(toa_bad_len_msg, "FC_TOA_INIT", packet->len, expected_size);
	  return;
  }

  if(packet->poll_addr[0] == settings.mac.addr) {
    // Tag is the target and you should send Init
    int ancCnt = packet->num_poll_anchor-1;
    TOA_SendInit(packet->poll_addr[ancCnt], packet->poll_addr, ancCnt);
  } else {
    // You are the target and you should send Poll
    // Poll will be sent as MAC frame in your slot
    // so next frame in slot won't be sent until rx timeout,
    // error occurrence or success ranging result
    TOA_SendPoll(packet->poll_addr, packet->num_poll_anchor);
  }
}

int FC_TOA_POLL_cb(const void* data, const prot_packet_info_t* info) {
  TOA_State(&toa.core, TOA_POLL_REC);
  FC_TOA_POLL_s* packet = (FC_TOA_POLL_s*)data;
  TOA_ASSERT(packet->FC == FC_TOA_POLL);
  PROT_CHECK_LEN(FC_TOA_POLL, packet->len, sizeof(FC_TOA_POLL_s));
  int64_t rx_ts = TRANSCEIVER_GetRxTimestamp();

  toa.core.TsPollRx = rx_ts;
  toa.core.initiator = info->direct_src;
  toa.core.anc_in_poll_cnt = packet->num_poll_anchor;
  toa.core.resp_ind = 0;
  const int addr_len = sizeof(dev_addr_t) * packet->num_poll_anchor;
  memcpy(toa.core.addr_tab, packet->poll_addr, addr_len);

  // check if it is full sync to you
  toa.core.resp_ind = TOA_FindAddrIndexInResp(&toa.core, settings.mac.addr);
  if (toa.core.resp_ind < toa.core.anc_in_poll_cnt) {
    int ret = TOA_SendResp(rx_ts);
    if (ret != 0) {
      LOG_DBG("TOA POLL->RESP tx timeout (%d)", MAC_UsFromRx());
      TRANSCEIVER_DefaultRx();
    } else {
      int dly = settings.mac.toa_dly.resp_dly_us[toa.core.resp_ind];
      int lag = MAC_UsFromRx();
      int tx_time = TRANSCEIVER_EstimateTxTimeUs(sizeof(FC_TOA_RESP_s));
      TOA_TRACE("TOA RESP sent to %X (%d>%d+%d)", toa.core.initiator, dly, lag,
                tx_time);
    }
  } else {
    // indicate that last poll wasn't to you
    toa.core.resp_ind = TOA_MAX_DEV_IN_POLL;
  }
  return 0;
}

int FC_TOA_RESP_cb(const void* data, const prot_packet_info_t* info) {
  TOA_State(&toa.core, TOA_RESP_REC);
  FC_TOA_RESP_s* packet = (FC_TOA_RESP_s*)data;
  TOA_ASSERT(packet->FC == FC_TOA_RESP);
  PROT_CHECK_LEN(FC_TOA_RESP, packet->len, sizeof(FC_TOA_RESP_s));
  int64_t rx_ts = TRANSCEIVER_GetRxTimestamp();

  if (toa.core.resp_ind < TOA_MAX_DEV_IN_POLL) {
    toa.core.TsRespRx[toa.core.resp_ind++] = rx_ts;
  }
  if (toa.core.resp_ind >= toa.core.anc_in_poll_cnt) {
    int ret = TOA_SendFinal();
    if (ret != 0) {
      TOA_TRACE("TOA RESP->FIN tx timeout (%d)", MAC_UsFromRx());
      TRANSCEIVER_DefaultRx();
    } else {
      int dly = settings.mac.toa_dly.fin_dly_us;
      int lag = MAC_UsFromRx();
      int tx_time = TRANSCEIVER_EstimateTxTimeUs(sizeof(FC_TOA_RESP_s));
      TOA_TRACE("TOA FIN sent to %X (%d>%d+%d)", toa.core.addr_tab[0], dly,
                lag, tx_time);
    }
  } else {
    int ret = TOA_EnableRxBeforeFin(&toa.core, &settings.mac.toa_dly,
                                    toa.core.TsPollRx);
    if (ret != 0) {
      int lag = MAC_UsFromRx();
      TOA_TRACE("TOA RESP->RESP rx timeout (%d, %d)", lag, toa.core.resp_ind);
    }
  }
  return 0;
}

int FC_TOA_FIN_cb(const void* data, const prot_packet_info_t* info) {
  TOA_State(&toa.core, TOA_FIN_REC);
  FC_TOA_FIN_s* packet = (FC_TOA_FIN_s*)data;
  int ts_len = toa.core.anc_in_poll_cnt * sizeof(*packet->TsRespRx);
  TOA_ASSERT(packet->FC == FC_TOA_FIN);
  PROT_CHECK_LEN(FC_TOA_FIN, packet->len, sizeof(FC_TOA_FIN_s) + ts_len);
  TOA_ASSERT(sizeof(*packet->TsRespRx) == sizeof(*toa.core.TsRespRx));
  TOA_ASSERT(sizeof(packet->TsFinTxBuf) == 5);

  // read timestamps
  int64_t TsFinRx40b = TRANSCEIVER_GetRxTimestamp();
  int64_t TsFinTx40b = TOA_Read40bValue(packet->TsFinTxBuf);
  int TsRespRxSize = sizeof(packet->TsRespRx[0]) * toa.core.anc_in_poll_cnt;
  memcpy(toa.core.TsRespRx, packet->TsRespRx, TsRespRxSize);
  toa.core.TsFinTx = TsFinTx40b;
  toa.core.TsPollTx = packet->TsPollTx;
  toa.core.TsFinRx = TsFinRx40b;

  if (toa.core.resp_ind < TOA_MAX_DEV_IN_POLL) {
    // when it was to you
    int tof_dw = TOA_CalcTofDwTu(&toa.core, toa.core.resp_ind);
    int dist_cm = TOA_TofToCm(tof_dw * DWT_TIME_UNITS);
    TOA_MeasurePushLocal(info->direct_src, dist_cm);
  } else {
    TOA_TRACE("Dist err");
  }

  // turn on receiver after FIN callback
  dwt_forcetrxoff();
  TRANSCEIVER_DefaultRx();
  return 0;
}

void FC_TOA_RES_cb(const void* data, const prot_packet_info_t* info) {
  FC_TOA_RES_s* packet = (FC_TOA_RES_s*)data;
  TOA_ASSERT(packet->FC == FC_TOA_RES);
  if (packet->len < sizeof(*packet)) {
	  LOG_ERR(toa_bad_len_msg, FC_TOA_FIN, packet->len, sizeof(*packet));
	  return;
  }
  TOA_MeasurePush(&packet->meas);
}

int TOA_RxCb(const void* data, const prot_packet_info_t* info) {
  int ret;
  switch (*(uint8_t*)data) {
    case FC_TOA_POLL:
      FC_TOA_POLL_cb(data, info);
      ret = 1;
      break;
    case FC_TOA_RESP:
      FC_TOA_RESP_cb(data, info);
      ret = 1;
      break;
    case FC_TOA_FIN:
      FC_TOA_FIN_cb(data, info);
      ret = 1;
      break;
    default:
      ret = 0;
      break;
  }
  return ret;
}

int TOA_TxCb(int64_t TsDwTx) {
  int ret = 0;
  switch (toa.core.state) {
    case TOA_POLL_WAIT_TO_SEND:
      TOA_State(&toa.core, TOA_POLL_SENT);
      toa.core.TsPollTx = TsDwTx;
      TOA_TRACE("TOA POLL sent");
      ret = 1;
      break;
    case TOA_RESP_WAIT_TO_SEND:
      TOA_State(&toa.core, TOA_RESP_SENT);
      toa.core.TsRespTx = TsDwTx;
      int resp_us = (toa.core.TsRespTx - toa.core.TsPollRx) / UUS_TO_DWT_TIME;
      TOA_TRACE("TOA RESP sent after %dus", resp_us);
      ret = 1;
      break;
    case TOA_FIN_WAIT_TO_SEND:
      TRANSCEIVER_DefaultRx();
      TOA_State(&toa.core, TOA_FIN_SENT);
      toa.core.TsFinTx = TsDwTx;
      int fin_us =
          ((TsDwTx - toa.core.TsPollTx) & MASK_40BIT) / UUS_TO_DWT_TIME;
      TOA_TRACE("TOA FIN sent after %dus from POLL", fin_us);
      ret = 0;  // to release transceiver
      break;
    default:
      ret = 0;
      break;
  }
  return ret;
}

int TOA_RxToCb() {
  // 2 - temp - to sync module - error - change to 1
  // 1 - to sync module
  // 0 - not to sync module
  int ret = 0;
  switch (toa.core.state) {
    case TOA_POLL_SENT:
    case TOA_RESP_REC:
      toa.core.TsRespRx[toa.core.resp_ind++] = 0;
      if (toa.core.resp_ind >= toa.core.anc_in_poll_cnt) {
        TOA_SendFinal();
        ret = 1;  // it was timeout to syn module
      } else {
        ret = 2;  // default rx to catch next resp
      }
      break;
    // if status is TOA_RESP_SENT and timeout arrive, then
    // there is some trouble with final message transmiting
    // so abort ranging
    case TOA_RESP_SENT: {
      int fin_to_dw = TRANSCEIVER_GetTime() - TRANSCEIVER_GetTxTimestamp();
      TOA_TRACE("TOA FIN TO after %d", fin_to_dw / UUS_TO_DWT_TIME);
      ret = 2;
      break;
    }
    default:
      if (toa.core.state != TOA_IDLE) {
        dwt_forcetrxoff();
        TRANSCEIVER_DefaultRx();
        TOA_State(&toa.core, TOA_IDLE);
      }
      break;
  }

  if (ret == 2) {
    // mac_default_rx_full();
    dwt_forcetrxoff();
    TRANSCEIVER_DefaultRx();
    TOA_State(&toa.core, TOA_IDLE);
    ret = 1;
  }
  return ret;
}
