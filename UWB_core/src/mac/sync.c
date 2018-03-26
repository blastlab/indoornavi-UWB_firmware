#include "sync.h"
#include "mac.h"

sync_instance_t sync;
extern mac_instance_t mac;

const char bad_len_msg[] = "%s bad len %d!=%d";
#define PROT_CHECK_LEN(FC, len, expected)           \
  \
do                                                \
  \
{                                                \
    if ((len) < (expected))                         \
    {                                               \
      LOG_ERR(bad_len_msg, #FC, (len), (expected)); \
      return -1;                                    \
    }                                               \
  \
}                                                \
  \
while(0)

#define MOVE_ARRAY_ELEM(ARR_EL, N) ARR_EL[N] = ARR_EL[N - 1]

int64_t sync_glob_time(int64_t dw_ts)
{
  int dt = dw_ts - sync.local_obj.update_ts;
  int64_t res = dw_ts + sync.local_obj.time_offset;
  res += sync.local_obj.time_coeffP[0] * dt;
  return res & MASK_40BIT;
}

// called from resp_cb
void toa_enable_rx_after_resp_cb(toa_settings_t *tset, toa_core_t *toa)
{
  int64_t rx_start = tset->resp_dly[toa->resp_ind] - tset->guard_time;
  if (toa->resp_ind == toa->anc_in_poll_cnt)
  {
    rx_start += tset->fin_dly;
  }
  rx_start = rx_start * DWT_TIME_UNITS + sync.toa_ts_poll_rx_raw;
  dwt_setdelayedtrxtime(rx_start & MASK_40BIT);
  dwt_setrxtimeout(2 * settings.mac.sync_dly.guard_time);
  dwt_rxenable(DWT_START_RX_DELAYED);
}

void sync_init_neightbour(sync_neightbour_t *neig, dev_addr_t addr,
                          int tree_level)
{
  memset(neig, 0, sizeof(*neig));
  neig->addr = addr;
  neig->tree_level = tree_level;
}

sync_neightbour_t *sync_find_or_create_neightbour(dev_addr_t addr,
                                                  int tree_level)
{
  sync_neightbour_t *neig;
  // search
  for (int i = 0; i < SYNC_MAC_NEIGHTBOURS; ++i)
  {
    neig = &sync.neightbour[i];
    if (neig->addr == addr)
    {
      return neig;
    }
  }
  // create
  for (int i = 0; i < SYNC_MAC_NEIGHTBOURS; ++i)
  {
    neig = &sync.neightbour[i];
    if (neig->addr == 0)
    {
      sync_init_neightbour(neig, addr, tree_level);
      return neig;
    }
  }
  return 0;
}

// convert (0):(64B) to (-20B):(+20B)
int64_t sync_trim_drift(int64_t drift)
{
  drift &= MASK_40BIT;
  if (drift > (MASK_40BIT >> 1))
  {
    drift -= MASK_40BIT + 1;
  }
  return drift;
}

// sigmoid function
float sync_smooth(float x) { return x / sqrtf(1.0f + x * x); }

// time coefficient P calculation
float sync_calc_time_coeff(sync_neightbour_t *neig)
{
  const float K = 0.5; // algorithm speed (0..1)
  float *X = neig->time_coeffP_raw;
  float *Y = neig->time_coeffP;
  float out = Y[1] + K * (X[0] - X[1]) + 0.1f * X[0];
  return sync_smooth(out);
}

// shift data arrays and calculate raw data
void sync_update_neightbour(sync_neightbour_t *neig, int64_t ext_time,
                            int64_t loc_time, int tof_dw)
{
  int64_t dt = (loc_time - neig->update_ts) & MASK_40BIT;
  if (tof_dw > 0)
  {
    neig->tof_dw = (neig->tof_dw + tof_dw) * 0.5f;
  }
  neig->time_offset += dt * neig->time_coeffP[0];
  neig->time_offset &= MASK_40BIT;

  int64_t drift = ext_time + (int64_t)neig->tof_dw - neig->time_offset;
  drift = sync_trim_drift(drift - sync_glob_time(loc_time));

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
  // neig->timeDriftSum += drift;
  neig->update_ts = loc_time;
}

void sync_update_local()
{
  // todo:
}

void sync_update(sync_neightbour_t *neig, int64_t ext_time, int64_t loc_time,
                 int tof_dw)
{
  sync_update_neightbour(neig, ext_time, loc_time, tof_dw);
  sync_update_local();

  if (tof_dw > 0 && settings.mac.raport_anchor_anchor_distance)
  {
    int distance = toa_tof_to_cm(tof_dw * DWT_TIME_UNITS);
    toa_add_measure(neig->addr, distance);
  }
}

int sync_send_poll(dev_addr_t dst, dev_addr_t anchors[], int anc_cnt)
{
  SYNC_ASSERT(0 < anc_cnt && anc_cnt < 8);
  SYNC_ASSERT(dst != ADDR_BROADCAST);
  mac_buf_t *buf = MAC_BufferPrepare(dst, false);
  int anc_addr_len = anc_cnt * sizeof(dev_addr_t);
  if (buf == 0)
  {
    return -1;
  }

  FC_SYNC_POLL_s packet = {
      .FC = FC_SYNC_POLL,
      .len = sizeof(FC_SYNC_POLL_s) + anc_cnt * sizeof(dev_addr_t),
      .num_poll_anchor = anc_cnt,
  };
  memcpy(&packet.poll_addr[0], &anchors[0], anc_addr_len);
  MAC_Write(buf, &packet, packet.len);

  sync.toa.addr_tab[0] = dst;
  memcpy(&sync.toa.addr_tab[1], anchors, anc_addr_len);

  // send this frame in your slot but with ranging flage
  buf->isRangingFrame = true;
  toa_state(&sync.toa, TOA_POLL_WAIT_TO_SEND);
  MAC_Send(buf, false);
  return 0;
}

int sync_send_resp(int64_t PollDwRxTs)
{
  toa_settings_t *tset = &settings.mac.sync_dly;
  int resp_dly = tset->resp_dly[sync.toa.resp_ind];

  sync.toa.TsRespTx = toa_set_tx_time(PollDwRxTs, resp_dly);
  sync.toa.TsRespTx = sync_glob_time(sync.toa.TsRespTx);

  mac_buf_t *buf = MAC_BufferPrepare(sync.toa.initiator, false);
  FC_SYNC_RESP_s packet = {
      .FC = FC_SYNC_RESP,
      .len = sizeof(FC_SYNC_RESP_s),
      .TsPollRx = sync.toa.TsPollRx,
      .TsRespTx = sync.toa.TsRespTx,
  };
  MAC_Write(buf, &packet, packet.len);

  int tx_to_rx_dly = tset->resp_dly[sync.toa.anc_in_poll_cnt];
  tx_to_rx_dly += tset->fin_dly - tset->guard_time - resp_dly;
  dwt_setrxaftertxdelay(tx_to_rx_dly * DWT_TIME_UNITS);
  dwt_setrxtimeout(2 * settings.mac.sync_dly.guard_time);

  toa_state(&sync.toa, TOA_RESP_WAIT_TO_SEND);
  const int tx_flags = DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED;
  return MAC_SendRangingResp(buf, tx_flags);
}

int sync_send_final()
{
  toa_settings_t *tset = &settings.mac.sync_dly;
  int fin_dly = tset->resp_dly[sync.toa.anc_in_poll_cnt];
  fin_dly += tset->fin_dly;
  int64_t TsFinTx = toa_set_tx_time(sync.toa.TsPollTx, fin_dly);

  mac_buf_t *buf = MAC_BufferPrepare(sync.toa.addr_tab[0], false);
  FC_SYNC_FIN_s packet = {
      .FC = FC_SYNC_FIN,
      .len = sizeof(FC_SYNC_FIN_s),
      .tree_level = sync.tree_level,
      .slot_num = mac.slot_number,
  };
  toa_write_40b_value(&packet.TsFinTxBuf[0], sync_glob_time(TsFinTx));

  int resp_rx_ts_len = sizeof(*packet.TsRespRx) * sync.toa.anc_in_poll_cnt;
  packet.len += resp_rx_ts_len;
  memcpy(&packet.TsRespRx[0], &sync.toa.TsRespRx[0], resp_rx_ts_len);
  MAC_Write(buf, &packet, packet.len);

  toa_state(&sync.toa, TOA_FIN_WAIT_TO_SEND);
  const int tx_flags = DWT_START_TX_DELAYED;
  return MAC_SendRangingResp(buf, tx_flags);
}

int FC_SYNC_POLL_cb(const void *data, const prot_packet_info_t *info)
{
  toa_state(&sync.toa, TOA_POLL_REC);
  FC_SYNC_POLL_s *packet = (FC_SYNC_POLL_s *)data;
  SYNC_ASSERT(packet->FC == FC_SYNC_FIN);
  PROT_CHECK_LEN(FC_SYNC_POLL, packet->len, sizeof(FC_SYNC_POLL_s));
  int64_t rx_ts = TRANSCEIVER_GetRxTimestamp();

  sync.toa_ts_poll_rx_raw = rx_ts;
  sync.toa.TsPollRx = sync_glob_time(rx_ts);
  sync.toa.initiator = info->direct_src;
  sync.toa.anc_in_poll_cnt = packet->num_poll_anchor;
  sync.toa.resp_ind = 0;
  const int addr_len = sizeof(dev_addr_t) * packet->num_poll_anchor;
  memcpy(sync.toa.addr_tab, packet->poll_addr, addr_len);

  // chech if it is full sync to you
  sync.toa.resp_ind = toa_find_addr_index_in_resp(&sync.toa, settings.mac.addr);
  if (sync.toa.resp_ind < sync.toa.anc_in_poll_cnt)
  {
    sync_send_resp(rx_ts);
  }
  else
  {
    // indicate that last poll wasn't to you
    sync.toa.resp_ind = TOA_MAX_DEV_IN_POLL;
  }
  return 0;
}

int FC_SYNC_RESP_cb(const void *data, const prot_packet_info_t *info)
{
  toa_state(&sync.toa, TOA_RESP_REC);
  FC_SYNC_FIN_s *packet = (FC_SYNC_FIN_s *)data;
  SYNC_ASSERT(packet->FC == FC_SYNC_FIN);
  PROT_CHECK_LEN(FC_SYNC_RESP, packet->len, sizeof(FC_SYNC_RESP_s));
  int64_t rx_ts = TRANSCEIVER_GetRxTimestamp();

  sync.toa.TsRespRx[sync.toa.resp_ind++] = sync_glob_time(rx_ts);
  if (sync.toa.resp_ind >= sync.toa.anc_in_poll_cnt)
  {
    sync_send_final();
  }
  else
  {
    toa_enable_rx_after_resp_cb(&settings.mac.sync_dly, &sync.toa);
  }
  return 0;
}

int FC_SYNC_FIN_cb(const void *data, const prot_packet_info_t *info)
{
  toa_state(&sync.toa, TOA_FIN_REC);
  FC_SYNC_FIN_s *packet = (FC_SYNC_FIN_s *)data;
  int ts_len = sync.toa.anc_in_poll_cnt * sizeof(*packet->TsRespRx);
  SYNC_ASSERT(packet->FC == FC_SYNC_FIN);
  PROT_CHECK_LEN(FC_SYNC_FIN, packet->len, sizeof(FC_SYNC_FIN_s) + ts_len);
  SYNC_ASSERT(sizeof(*packet->TsRespRx) == sizeof(*sync.toa.TsRespRx));
  SYNC_ASSERT(sizeof(packet->TsFinTxBuf) == 5);

  sync_neightbour_t *neig =
      sync_find_or_create_neightbour(info->direct_src, packet->tree_level);
  if (neig == 0)
  {
    return 0;
  }

  // read timestamps
  sync.toa.TsPollTx = packet->TsPollTx;
  sync.toa.TsRespRx[sync.toa.resp_ind] = packet->TsRespRx[sync.toa.resp_ind];
  sync.toa.TsFinRx = sync_glob_time(TRANSCEIVER_GetRxTimestamp());
  int64_t ext_time = toa_read_40b_value(packet->TsFinTxBuf);
  int64_t loc_time = sync.toa.TsFinRx;

  if (sync.toa.resp_ind < TOA_MAX_DEV_IN_POLL)
  {
    // when it was to you
    int tof_dw = toa_calc_tof_dw_tu(&sync.toa, sync.toa.resp_ind);
    sync_update(neig, ext_time, loc_time, tof_dw);
  }
  else
  {
    // todo: sync_brief(&sync.toa, neig);
    sync_update(neig, ext_time, loc_time, 0);
  }
  return 0;
}

int sync_tx_cb(int64_t TsDwTx)
{
  int ret = 0;
  switch (sync.toa.state)
  {
  case TOA_POLL_WAIT_TO_SEND:
    toa_state(&sync.toa, TOA_POLL_SENT);
    sync.toa.TsPollTx = sync_glob_time(TsDwTx);
    ret = 1;
    break;
  case TOA_RESP_WAIT_TO_SEND:
    toa_state(&sync.toa, TOA_RESP_SENT);
    sync.toa.TsRespTx = sync_glob_time(TsDwTx);
    ret = 1;
    break;
  case TOA_FIN_WAIT_TO_SEND:
    toa_state(&sync.toa, TOA_FIN_SENT);
    sync.toa.TsFinTx = sync_glob_time(TsDwTx);
    ret = 1;
    break;
  default:
    ret = 0;
    break;
  }
  return ret;
}

int sync_rx_to_cb(int64_t Ts)
{
  // 2 - temp - to sync module - error - change to 1
  // 1 - to sync module
  // 0 - not to sync module
  int ret = 0;
  switch (sync.toa.state)
  {
  case TOA_POLL_SENT:
  case TOA_RESP_REC:
    sync.toa.TsRespRx[sync.toa.resp_ind++] = 0;
    if (sync.toa.resp_ind >= sync.toa.anc_in_poll_cnt)
    {
      sync_send_final();
      ret = 1; // it was timeout to syn module
    }
    else
    {
      ret = 2; // default rx to catch next resp
    }
    break;
  // if status is TOA_RESP_SENT and timeout arrive, then
  // there is some trouble with final message transmiting
  // so abort ranging
  case TOA_RESP_SENT:
    toa_state(&sync.toa, TOA_IDLE);
    ret = 2;
    break;
  default:
    break;
  }

  if (ret == 2)
  {
    // mac_default_rx_full();
    ret = 1;
  }
  return ret;
}
