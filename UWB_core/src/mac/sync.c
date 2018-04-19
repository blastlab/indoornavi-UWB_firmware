#include "sync.h"
#include "mac.h"

sync_instance_t sync;
extern mac_instance_t mac;

const char bad_len_msg[] = "%s bad len %d!=%d";
#define PROT_CHECK_LEN(FC, len, expected)                                      \
  \
do \
{                                                                        \
    if ((len) < (expected)) {                                                  \
      LOG_ERR(bad_len_msg, #FC, (len), (expected));                            \
      return -1;                                                               \
    }                                                                          \
  \
}                                                                         \
  \
while(0)

#define MOVE_ARRAY_ELEM(ARR_EL, N) ARR_EL[N] = ARR_EL[N - 1]

void SYNC_Init() {
  toa_settings_t *tset = &settings.mac.sync_dly;
  const int rx_to_tx_delay = 30; // us
  const float spi_speed = 20e6;  // Hz
  const int POLL_PROCESSING_TIME_US = 270;
  const int RESP_PROCESSING_TIME_US = 200;

  tset->guard_time_us = 50; // us
  tset->rx_after_tx_offset_us = tset->guard_time_us + TRANSCEIVER_EstimateTxTimeUs(0);

  // add here 2 bytes for CRC
  int basePollLen = MAC_HEAD_LENGTH + sizeof(FC_SYNC_POLL_s) + (2);
  basePollLen += sizeof(dev_addr_t) * TOA_MAX_DEV_IN_POLL;
  const int respLen = MAC_HEAD_LENGTH + sizeof(FC_SYNC_RESP_s) + (2);
  const int baseFinLen = MAC_HEAD_LENGTH + sizeof(FC_SYNC_FIN_s) + (2);
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

int64_t SYNC_GlobTime(int64_t dw_ts) {
  int dt = dw_ts - sync.local_obj.update_ts;
  int64_t res = dw_ts + sync.local_obj.time_offset;
  res += sync.local_obj.time_coeffP[0] * dt;
  return res & MASK_40BIT;
}

void SYNC_InitNeightbour(sync_neightbour_t *neig, dev_addr_t addr,
                         int tree_level) {
  memset(neig, 0, sizeof(*neig));
  neig->addr = addr;
  neig->tree_level = tree_level;
}

sync_neightbour_t *SYNC_FindOrCreateNeightbour(dev_addr_t addr,
                                               int tree_level) {
  sync_neightbour_t *neig;
  // search
  for (int i = 0; i < SYNC_MAC_NEIGHTBOURS; ++i) {
    neig = &sync.neightbour[i];
    if (neig->addr == addr) {
      return neig;
    }
  }
  // create
  for (int i = 0; i < SYNC_MAC_NEIGHTBOURS; ++i) {
    neig = &sync.neightbour[i];
    if (neig->addr == 0) {
      SYNC_InitNeightbour(neig, addr, tree_level);
      return neig;
    }
  }
  return 0;
}

// convert (0):(64B) to (-20B):(+20B)
int64_t SYNC_TrimDrift(int64_t drift) {
  drift &= MASK_40BIT;
  if (drift > (MASK_40BIT >> 1)) {
    drift -= MASK_40BIT + 1;
  }
  return drift;
}

// sigmoid function
float SYNC_Smooth(float x) { return x / sqrtf(1.0f + x * x); }

// time coefficient P calculation
float SYNC_CalcTimeCoeff(sync_neightbour_t *neig) {
  const float K = 0.5; // algorithm speed (0..1)
  float *X = neig->time_coeffP_raw;
  float *Y = neig->time_coeffP;
  float out = Y[1] + K * (X[0] - X[1]) + 0.1f * X[0];
  return SYNC_Smooth(out);
}

// shift data arrays and calculate raw data
void SYNC_UpdateNeightbour(sync_neightbour_t *neig, int64_t ext_time,
                           int64_t loc_time, int tof_dw) {
  int64_t dt = (loc_time - neig->update_ts) & MASK_40BIT;
  if (tof_dw > 0) {
    neig->tof_dw = (neig->tof_dw + tof_dw) * 0.5f;
  }
  neig->time_offset += dt * neig->time_coeffP[0];
  neig->time_offset &= MASK_40BIT;

  int64_t drift = ext_time + (int64_t)neig->tof_dw - neig->time_offset;
  drift = SYNC_TrimDrift(drift - SYNC_GlobTime(loc_time));

  MOVE_ARRAY_ELEM(neig->drift, 2);
  MOVE_ARRAY_ELEM(neig->drift, 1);
  MOVE_ARRAY_ELEM(neig->time_coeffP_raw, 2);
  MOVE_ARRAY_ELEM(neig->time_coeffP_raw, 1);
  MOVE_ARRAY_ELEM(neig->time_coeffP, 1);
  neig->drift[0] = drift;
  neig->time_coeffP_raw[0] =
      dt != 0.0 ? ((float)drift) / ((float)dt) : neig->time_coeffP_raw[1];
  neig->time_coeffP_raw[0] =
      isnanf(neig->time_coeffP_raw[0]) ? 0 : neig->time_coeffP_raw[0];
  neig->time_coeffP[0] = SYNC_CalcTimeCoeff(neig);
  // neig->timeDriftSum += drift;
  neig->update_ts = loc_time;
}

// only for UpdateLocalTimeParams
int64_t MAC_ToSlotsTime(int64_t transceiver_raw_time);

void SYNC_UpdateLocalTimeParams(uint64_t transceiver_raw_time) {
  // todo: update SYNC hr time
  // try update local
  if (transceiver_raw_time - sync.local_obj.update_ts > 20 * UUS_TO_DWT_TIME) {
    float sum_P = 0;
    int cnt = 0;
    int64_t offset = 0;
    for (int i = 0; i < SYNC_MAC_NEIGHTBOURS; ++i) {
      if (sync.neightbour[i].update_ts) {
        sum_P += sync.neightbour[i].time_coeffP[0];
        offset += sync.neightbour[i].time_offset; // 63b / 40b = 23b
        ++cnt;
      }
    }
    SYNC_UpdateNeightbour(&sync.local_obj, offset / cnt,
                          sync.local_obj.time_offset, 0);
    float a = 0.5;
    sync.local_obj.time_coeffP[0] =
        a * sync.local_obj.time_coeffP[0] + (1.0f - a) * sum_P / cnt;
  }
  // update MAC slot timer
  int64_t slot_time = MAC_ToSlotsTime(transceiver_raw_time);
  int left_time =
      settings.mac.slot_number * settings.mac.slot_time_us - slot_time;
  PORT_SlotTimerSetUsLeft(left_time * DWT_TIME_UNITS * 1.0e6f);
}

// complex syncronisation processing
void SYNC_Update(sync_neightbour_t *neig, int64_t ext_time, int64_t loc_time,
                 int tof_dw) {
  //SYNC_UpdateNeightbour(neig, ext_time, loc_time, tof_dw);
  //SYNC_UpdateLocalTimeParams(loc_time);

  // add distance to measure table
  if (tof_dw > 0 && settings.mac.raport_anchor_anchor_distance) {
    int distance = TOA_TofToCm(tof_dw * DWT_TIME_UNITS);
    TOA_AddMeasure(neig->addr, distance);
  }
}

int SYNC_SendPoll(dev_addr_t dst, dev_addr_t anchors[], int anc_cnt) {
  SYNC_ASSERT(0 < anc_cnt && anc_cnt < 8);
  SYNC_ASSERT(dst != ADDR_BROADCAST);
  mac_buf_t *buf = MAC_BufferPrepare(dst, false);
  int anc_addr_len = anc_cnt * sizeof(dev_addr_t);
  if (buf == 0) {
    return -1;
  }

  FC_SYNC_POLL_s packet = {
      .FC = FC_SYNC_POLL,
      .len = sizeof(FC_SYNC_POLL_s) + anc_cnt * sizeof(dev_addr_t),
      .num_poll_anchor = anc_cnt,
  };
  memcpy(&packet.poll_addr[0], &anchors[0], anc_addr_len);
  MAC_Write(buf, &packet, packet.len);

  sync.toa.resp_ind = 0;
  sync.toa.anc_in_poll_cnt = anc_cnt;
  sync.toa.initiator = settings.mac.addr;
  sync.toa.addr_tab[0] = dst;
  memcpy(&sync.toa.addr_tab[1], anchors, anc_addr_len);

  // send this frame in your slot (through queue) but with ranging flage
  // mac module should set DWT_RESPONSE_EXPECTED flag before transmission
  buf->isRangingFrame = true;
  TOA_State(&sync.toa, TOA_POLL_WAIT_TO_SEND);
  MAC_Send(buf, false);
  return 0;
}

int SYNC_SendResp(int64_t PollDwRxTs) {
  toa_settings_t *tset = &settings.mac.sync_dly;
  int resp_dly_us = tset->resp_dly_us[sync.toa.resp_ind];

  sync.toa.TsRespTx = TOA_SetTxTime(PollDwRxTs, resp_dly_us);
  sync.toa.TsRespTx = SYNC_GlobTime(sync.toa.TsRespTx);

  mac_buf_t *buf = MAC_BufferPrepare(sync.toa.initiator, false);
  if (buf == 0) {
    TOA_State(&sync.toa, TOA_IDLE);
    return 0;
  }
  FC_SYNC_RESP_s packet = {
      .FC = FC_SYNC_RESP,
      .len = sizeof(FC_SYNC_RESP_s),
      .TsPollRx = sync.toa.TsPollRx,
      .TsRespTx = sync.toa.TsRespTx,
  };
  MAC_Write(buf, &packet, packet.len);

  int tx_to_rx_dly_us = tset->resp_dly_us[sync.toa.anc_in_poll_cnt-1];
  tx_to_rx_dly_us += tset->fin_dly_us - resp_dly_us;
  dwt_setrxaftertxdelay(tx_to_rx_dly_us - tset->rx_after_tx_offset_us);
  dwt_setrxtimeout(settings.mac.sync_dly.guard_time_us + tset->rx_after_tx_offset_us);

  TOA_State(&sync.toa, TOA_RESP_WAIT_TO_SEND);
  const int tx_flags = DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED;
  return MAC_SendRanging(buf, tx_flags);
}

int SYNC_SendFinal() {
  toa_settings_t *tset = &settings.mac.sync_dly;
  int fin_dly_us = tset->resp_dly_us[sync.toa.anc_in_poll_cnt-1];
  fin_dly_us += tset->fin_dly_us;
  int64_t TsFinTx = TOA_SetTxTime(sync.toa.TsPollTx, fin_dly_us);

  mac_buf_t *buf = MAC_BufferPrepare(sync.toa.addr_tab[0], false);
  FC_SYNC_FIN_s packet = {
      .FC = FC_SYNC_FIN,
      .len = sizeof(FC_SYNC_FIN_s),
      .tree_level = sync.tree_level,
      .slot_num = mac.slot_number,
			.TsPollTx = sync.toa.TsPollTx,
  };
  // calc global time in place to keep 40B resolution
  TOA_Write40bValue(&packet.TsFinTxBuf[0], SYNC_GlobTime(TsFinTx));

  int resp_rx_ts_len = sizeof(*packet.TsRespRx) * sync.toa.anc_in_poll_cnt;
  packet.len += resp_rx_ts_len;
  memcpy(&packet.TsRespRx[0], &sync.toa.TsRespRx[0], resp_rx_ts_len);
  MAC_Write(buf, &packet, packet.len);

  TOA_State(&sync.toa, TOA_FIN_WAIT_TO_SEND);
  const int tx_flags = DWT_START_TX_DELAYED;
  return MAC_SendRanging(buf, tx_flags);
}

int FC_SYNC_POLL_cb(const void *data, const prot_packet_info_t *info) {
  TOA_State(&sync.toa, TOA_POLL_REC);
  FC_SYNC_POLL_s *packet = (FC_SYNC_POLL_s *)data;
  SYNC_ASSERT(packet->FC == FC_SYNC_FIN);
  PROT_CHECK_LEN(FC_SYNC_POLL, packet->len, sizeof(FC_SYNC_POLL_s));
  int64_t rx_ts = TRANSCEIVER_GetRxTimestamp();

  sync.toa_ts_poll_rx_raw = rx_ts;
  sync.toa.TsPollRx = SYNC_GlobTime(rx_ts);
  sync.toa.initiator = info->direct_src;
  sync.toa.anc_in_poll_cnt = packet->num_poll_anchor;
  sync.toa.resp_ind = 0;
  const int addr_len = sizeof(dev_addr_t) * packet->num_poll_anchor;
  memcpy(sync.toa.addr_tab, packet->poll_addr, addr_len);

  // chech if it is full sync to you
  sync.toa.resp_ind = TOA_FindAddrIndexInResp(&sync.toa, settings.mac.addr);
  if (sync.toa.resp_ind < sync.toa.anc_in_poll_cnt) {
    int ret = SYNC_SendResp(rx_ts);
    if (ret != 0) {
      LOG_DBG("Sync POLL->RESP tx timeout (%d)", MAC_UsFromRx());
    } else {
      int dly = settings.mac.sync_dly.resp_dly_us[sync.toa.resp_ind];
      int lag = MAC_UsFromRx();
      int tx_us = TRANSCEIVER_EstimateTxTimeUs(sizeof(FC_SYNC_RESP_s));
      SYNC_TRACE("SYNC RESP sent to %X (%d>%d+%d)", sync.toa.initiator, dly, lag,
              tx_us);
    }
  } else {
    // indicate that last poll wasn't to you
    sync.toa.resp_ind = TOA_MAX_DEV_IN_POLL;
  }
  return 0;
}

int FC_SYNC_RESP_cb(const void *data, const prot_packet_info_t *info) {
  TOA_State(&sync.toa, TOA_RESP_REC);
  FC_SYNC_RESP_s *packet = (FC_SYNC_RESP_s *)data;
  SYNC_ASSERT(packet->FC == FC_SYNC_RESP);
  PROT_CHECK_LEN(FC_SYNC_RESP, packet->len, sizeof(FC_SYNC_RESP_s));
  int64_t rx_ts = TRANSCEIVER_GetRxTimestamp();

  sync.toa.TsRespRx[sync.toa.resp_ind++] = SYNC_GlobTime(rx_ts);
  if (sync.toa.resp_ind >= sync.toa.anc_in_poll_cnt) {
    int ret = SYNC_SendFinal();
    if (ret != 0) {
      SYNC_TRACE("Sync RESP->FIN tx timeout (%d)", MAC_UsFromRx());
    } else {
      int dly = settings.mac.sync_dly.fin_dly_us;
      int lag = MAC_UsFromRx();
      int tx_us = TRANSCEIVER_EstimateTxTimeUs(sizeof(FC_SYNC_RESP_s));
      SYNC_TRACE("SYNC FIN sent to %X (%d>%d+%d)", sync.toa.addr_tab[0], dly, lag,
              tx_us);
    }
  } else {
    int ret = TOA_EnableRxBeforeFin(&sync.toa, &settings.mac.sync_dly,
                                    sync.toa_ts_poll_rx_raw);
    if (ret != 0) {
    	SYNC_TRACE("Sync RESP->RESP rx timeout (%d, %d)", MAC_UsFromRx(),
              sync.toa.resp_ind);
    }
  }
  return 0;
}

int FC_SYNC_FIN_cb(const void *data, const prot_packet_info_t *info) {
  TOA_State(&sync.toa, TOA_FIN_REC);
  FC_SYNC_FIN_s *packet = (FC_SYNC_FIN_s *)data;
  int ts_len = sync.toa.anc_in_poll_cnt * sizeof(*packet->TsRespRx);
  SYNC_ASSERT(packet->FC == FC_SYNC_FIN);
  PROT_CHECK_LEN(FC_SYNC_FIN, packet->len, sizeof(FC_SYNC_FIN_s) + ts_len);
  SYNC_ASSERT(sizeof(*packet->TsRespRx) == sizeof(*sync.toa.TsRespRx));
  SYNC_ASSERT(sizeof(packet->TsFinTxBuf) == 5);

  sync_neightbour_t *neig =
      SYNC_FindOrCreateNeightbour(info->direct_src, packet->tree_level);
  if (neig == 0) {
    return 0;
  }

  // read timestamps
  uint64_t TsFinTx40b =TOA_Read40bValue(packet->TsFinTxBuf);
  memcpy(sync.toa.TsRespRx, packet->TsRespRx, sizeof(packet->TsRespRx[0])*sync.toa.anc_in_poll_cnt);
  sync.toa.TsFinTx = TsFinTx40b;
  sync.toa.TsPollTx = packet->TsPollTx;
  sync.toa.TsFinRx = SYNC_GlobTime(TRANSCEIVER_GetRxTimestamp());

  int64_t ext_time = TsFinTx40b;
  int64_t loc_time = sync.toa.TsFinRx;

  if (sync.toa.resp_ind < TOA_MAX_DEV_IN_POLL) {
    // when it was to you
    int tof_dw = TOA_CalcTofDwTu(&sync.toa, sync.toa.resp_ind);
    SYNC_Update(neig, ext_time, loc_time, tof_dw);
    LOG_DBG("Dist %d", TOA_TofToCm(tof_dw * DWT_TIME_UNITS));
  } else {
    // todo: sync_brief(&sync.toa, neig);
    SYNC_Update(neig, ext_time, loc_time, 0);
    LOG_DBG("Dist err");
  }

  // turn on receiver after FIN callback
  dwt_forcetrxoff();
  TRANSCEIVER_DefaultRx();
  return 0;
}

int SYNC_RxCb(const void *data, const prot_packet_info_t *info) {
  int ret;
  switch (*(uint8_t *)data) {
  case FC_SYNC_POLL:
    FC_SYNC_POLL_cb(data, info);
    ret = 1;
    break;
  case FC_SYNC_RESP:
    FC_SYNC_RESP_cb(data, info);
    ret = 1;
    break;
  case FC_SYNC_FIN:
    FC_SYNC_FIN_cb(data, info);
    ret = 1;
    break;
  default:
    ret = 0;
    break;
  }
  return ret;
}

int SYNC_TxCb(int64_t TsDwTx) {
  int ret = 0;
  switch (sync.toa.state) {
  case TOA_POLL_WAIT_TO_SEND:
    TOA_State(&sync.toa, TOA_POLL_SENT);
    sync.toa.TsPollTx = SYNC_GlobTime(TsDwTx);
    SYNC_TRACE("SYNC POLL sent");
    ret = 1;
    break;
  case TOA_RESP_WAIT_TO_SEND:
    TOA_State(&sync.toa, TOA_RESP_SENT);
    sync.toa.TsRespTx = SYNC_GlobTime(TsDwTx);
    int resp_us = (sync.toa.TsRespTx-sync.toa.TsPollRx) / UUS_TO_DWT_TIME;
    SYNC_TRACE("SYNC RESP sent after %dus", resp_us);
    ret = 1;
    break;
  case TOA_FIN_WAIT_TO_SEND:
    TOA_State(&sync.toa, TOA_FIN_SENT);
    sync.toa.TsFinTx = SYNC_GlobTime(TsDwTx);
    int fin_us = (sync.toa.TsFinTx-sync.toa.TsPollTx) / UUS_TO_DWT_TIME;
    SYNC_TRACE("SYNC FIN sent after %dus from POLL", fin_us);
    ret = 0; // to release transceiver
    break;
  default:
    ret = 0;
    break;
  }
  return ret;
}

int SYNC_RxToCb() {
  // 2 - temp - to sync module - error - change to 1
  // 1 - to sync module
  // 0 - not to sync module
  int ret = 0;
  switch (sync.toa.state) {
  case TOA_POLL_SENT:
  case TOA_RESP_REC:
    sync.toa.TsRespRx[sync.toa.resp_ind++] = 0;
    if (sync.toa.resp_ind >= sync.toa.anc_in_poll_cnt) {
      SYNC_SendFinal();
      ret = 1; // it was timeout to syn module
    } else {
      ret = 2; // default rx to catch next resp
    }
    break;
  // if status is TOA_RESP_SENT and timeout arrive, then
  // there is some trouble with final message transmiting
  // so abort ranging
  case TOA_RESP_SENT:
  	SYNC_TRACE("SYNC FIN TO after %d", (TRANSCEIVER_GetTime() - TRANSCEIVER_GetTxTimestamp()) / UUS_TO_DWT_TIME);
    ret = 2;
    break;
  default:
  	if(sync.toa.state != TOA_IDLE) {
  		dwt_forcetrxoff();
  		TRANSCEIVER_DefaultRx();
  		TOA_State(&sync.toa, TOA_IDLE);
  	}
    break;
  }

  if (ret == 2) {
    // mac_default_rx_full();
  	dwt_forcetrxoff();
  	TRANSCEIVER_DefaultRx();
    TOA_State(&sync.toa, TOA_IDLE);
    ret = 1;
  }
  return ret;
}
