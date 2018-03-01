#include "sync.h"

sync_instance_t sync;

uint64_t sync_set_tx_time(uint64_t dw_time, uint32_t delay_us)
{
    dw_time += delay_us * UUS_TO_DWT_TIME;
    dw_time &= 0x00FFFFFFFE00;           // trim value to tx timer resolution
    dwt_setdelayedtrxtime(dw_time >> 8); // convert to tx timer unit
    return (dw_time + settings.transceiver.ant_dly_tx) & MASK_40BIT;
}

int FC_SYNC_POLL_cb(const void *data, const void *prot_packet_info_t)
{
    FC_SYNC_POLL_s *packet = (FC_SYNC_POLL_s *)data;
    SYNC_ASSERT(packet->FC == FC_SYNC_FIN);
}

int FC_SYNC_RESP_cb(const void *data, const void *prot_packet_info_t)
{
    FC_SYNC_FIN_s *packet = (FC_SYNC_FIN_s *)data;
    SYNC_ASSERT(packet->FC == FC_SYNC_FIN);
}

int FC_SYNC_FIN_cb(const void *data, const void *prot_packet_info_t)
{
    FC_SYNC_FIN_s *packet = (FC_SYNC_FIN_s *)data;
    SYNC_ASSERT(packet->FC == FC_SYNC_FIN);
}