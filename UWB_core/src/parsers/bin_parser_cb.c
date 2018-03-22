#include "bin_parser.h"

void _bin_finalize(uint8_t FC, const void *data, uint8_t len, const void *prot_packet_info_t)
{

    if(info->direct_src == ADDR_BROADCAST)
    {
        print_stat(&packet, settings.mac.addr);
    }
    else
    {
        uint8_t *header = (uint8_t*)data;
        header[0] = FC;
        header[1] = len;
        mac_buf_t *buf = carry_prepare_buf_to(inf->direct_src);
        mac_write(buf, &packet, packet->len);
        carry_send(buf, false);
    }
}

int FC_STAT_ASK_cb(const void *data, const void *prot_packet_info_t info)
{
    BIN_ASSERT(*(uint8_t*)data == FC_STAT_ASK);
    FC_STAT_s packet;
    packet.err_cnt = 1;
    packet.rx_cnt = 2;
    _bin_finalize(FC_STAT_RESP, &packet, packet.len, info);
}

const prot_cb_t prot_cb_tab[] = {
    {FC_CARRY, 0},
    {FC_STAT_ASK, FC_STAT_ASK_cb},
};
const int prot_cb_len = sizeof(prot_cb_tab) / sizeof(*prot_cb_tab);