#include "sync.h"
#include "mac/mac.h"

sync_instance_t sync;
extern mac_instance_t mac;

void toa_state(toa_core_t *toa, toa_state_t state)
{
    toa->state = state;
}

uint64_t sync_set_tx_time(uint64_t dw_time, uint32_t delay_us)
{
    dw_time += delay_us * UUS_TO_DWT_TIME;
    dw_time &= 0x00FFFFFFFE00;           // trim value to tx timer resolution
    dwt_setdelayedtrxtime(dw_time >> 8); // convert to tx timer unit
    return (dw_time + settings.transceiver.ant_dly_tx) & MASK_40BIT;
}

void sync_write_40b_value(uint8_t *dst, uint64_t val)
{
    for (int i = 0; i < 5 * 8; i += 8)
    {
        *dst = 0xFF & (val >> i);
        ++dst;
    }
}

uint64_t sync_read_40b_value(uint8_t *src)
{
    uint64_t val = 0;
    for (int i = 0; i < 5 * 8; i += 8)
    {
        val = (val << i) + *src;
        ++src;
    }
}

uint64_t sync_glob_time(uint64_t dw_ts)
{
    int dt = dw_ts - sync.local_obj.update_ts;
    int64_t res = dw_ts + sync.local_obj.time_offset[0];
    res += sync.local_obj.time_coeffP[0] * dt;
    return res & MASK_40BIT;
}

int toa_find_resp_ind(toa_core_t *toa)
{
    int ind = 0;
    while (ind < toa->anc_in_poll_cnt)
    {
        if (toa->addr_tab[ind] == settings.mac.addr)
        {
            return ind;
        }
        ++ind;
    }
    return MAC_SYNC_MAX_AN_CNT;
}

int sync_send_poll(dev_addr_t dst, dev_addr_t anchors[], int anc_cnt)
{
    SYNC_ASSERT(0 < anc_cnt && anc_cnt < 8);
    SYNC_ASSERT(dst != ADDR_BROADCAST);
    mac_buf_t *buf = mac_buffer_prepare(dst, false);
    if (buf == 0)
    {
        return -1;
    }

    FC_SYNC_POLL_s packet = {
        .FC = FC_SYNC_POLL,
        .len = sizeof(FC_SYNC_POLL_s) + anc_cnt * sizeof(dev_addr_t),
        .tree_level = sync.tree_level,
        .slot_num = mac.slot_number,
        .num_poll_anchor = anc_cnt,
    };
    memcpy(anchors, &packet.poll_addr[0], anc_cnt * sizeof(anc_cnt));
    mac_write(buf, &packet, packet.len);

    sync.toa.addr_tab[0] = dst;
    memcpy(&sync.toa.addr_tab[1], anchors, anc_cnt * sizeof(dev_addr_t));

    // send this frame in your slot but with ranging flage
    buf->isRangingFrame = true;
    toa_state(&sync.toa, TOA_POLL_WAIT_TO_SEND);
    mac_send(buf, false);
    return 0;
}

int sync_send_resp(uint64_t PollDwRxTs)
{
    toa_settings_t *tset = &settings.mac.sync_dly;
    int resp_dly = tset->resp_dly[sync.toa.resp_ind];
    int tx_to_rx_dly = tset->resp_dly[sync.toa.anc_in_poll_cnt];
    tx_to_rx_dly += tset->fin_dly - tset->guard_time - resp_dly;

    mac_buf_t *buf = mac_buffer_prepare(sync.toa.initiator, false);
    FC_SYNC_RESP_s packet = {
        .FC = FC_SYNC_RESP,
        .len = sizeof(FC_SYNC_RESP_s),
        .TsPollRx = sync.toa.TsPollRx,
        .TsRespTx = sync_glob_time(sync_set_tx_time(PollDwRxTs, resp_dly)),
    };
    mac_write(buf, &packet, packet.len);

    dwt_setrxaftertxdelay(tx_to_rx_dly * DWT_TIME_UNITS);
    dwt_setrxtimeout(2 * settings.mac.sync_dly.guard_time);
    toa_state(&sync.toa, TOA_RESP_WAIT_TO_SEND);
    const int tx_flags = DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED;
    return mac_send_ranging_resp(buf, tx_flags);
}

int sync_send_final()
{
    toa_settings_t *tset = &settings.mac.sync_dly;
    int fin_dly = tset->resp_dly[sync.toa.anc_in_poll_cnt];
    fin_dly += tset->fin_dly;

    mac_buf_t *buf = mac_buffer_prepare(sync.toa.addr_tab[0], false);
    FC_SYNC_FIN_s packet = {
        .FC = FC_SYNC_FIN,
        .len = sizeof(FC_SYNC_FIN_s),
        .TsFinTx = sync_glob_time(sync_set_tx_time(sync.toa.TsPollTx, fin_dly)),
    };
    int resp_rx_ts_len = sizeof(*packet.TsRespRx) * sync.toa.anc_in_poll_cnt;
    packet.len += resp_rx_ts_len;
    memcpy(&packet.TsRespRx[0], &sync.toa.TsRespRx[0], resp_rx_ts_len);
    mac_write(buf, &packet, packet.len);

    toa_state(&sync.toa, TOA_FIN_WAIT_TO_SEND);
    const int tx_flags = DWT_START_TX_DELAYED;
    return mac_send_ranging_resp(buf, tx_flags);
}

int FC_SYNC_POLL_cb(const void *data, const prot_packet_info_t *info)
{
    toa_state(&sync.toa, TOA_POLL_REC);
    FC_SYNC_POLL_s *packet = (FC_SYNC_POLL_s *)data;
    SYNC_ASSERT(packet->FC == FC_SYNC_FIN);
    uint64_t rx_ts = transceiver_get_rx_timestamp();

    sync.toa_ts_poll_rx_raw = rx_ts;
    sync.toa.TsPollRx = sync_glob_time(rx_ts);
    sync.toa.initiator = info->direct_src;
    sync.toa.anc_in_poll_cnt = packet->num_poll_anchor;
    sync.toa.resp_ind = 0;
    const int addr_len = sizeof(dev_addr_t) * packet->num_poll_anchor;
    memcpy(sync.toa.addr_tab, packet->poll_addr, addr_len);

    // chech if it is full sync to you
    sync.toa.resp_ind = toa_find_resp_ind(&sync.toa);
    if (sync.toa.resp_ind < sync.toa.anc_in_poll_cnt)
    {
        sync_send_resp(rx_ts);
    }
}

int FC_SYNC_RESP_cb(const void *data, const prot_packet_info_t *info)
{
    FC_SYNC_FIN_s *packet = (FC_SYNC_FIN_s *)data;
    SYNC_ASSERT(packet->FC == FC_SYNC_FIN);
}

int FC_SYNC_FIN_cb(const void *data, const prot_packet_info_t *info)
{
    FC_SYNC_FIN_s *packet = (FC_SYNC_FIN_s *)data;
    SYNC_ASSERT(packet->FC == FC_SYNC_FIN);
}

int sync_tx_cb(uint64_t TsDwTx)
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
        sync.toa.TsRespTx = sync_glob_time(TsDwTx);
        ret = 1;
        break;
    default:
        ret = 0;
        break;
    }
    return ret;
}

int sync_rx_to_cb(uint64_t Ts)
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
    }

    if (ret == 2)
    {
        //mac_default_rx_full();
        ret = 1;
    }
    return ret;
}