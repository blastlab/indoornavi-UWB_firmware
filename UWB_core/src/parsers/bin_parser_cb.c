#include "../mac/carry.h"
#include "bin_parser.h"
#include "bin_struct.h"
#include "printer.h"


void _BIN_Finalize(uint8_t FC, const void *data, uint8_t len,
                   const prot_packet_info_t *info) {
  if (info->direct_src == ADDR_BROADCAST) {
    FC_STAT_s stat;
    stat.err_cnt = 1;
    stat.to_cnt = 2;
    stat.rx_cnt = 3;
    PRINT_Stat(&stat, settings.mac.addr);
  } else {
    uint8_t *header = (uint8_t *)data;
    header[0] = FC;
    header[1] = len;
    mac_buf_t *buf = CARRY_PrepareBufTo(info->direct_src);
    MAC_Write(buf, data, len);
    MAC_Send(buf, false);
  }
}

void FC_SYNC_POLL_BIN_cb(const void *data, const prot_packet_info_t *info) {
  FC_SYNC_POLL_cb(data, info);
}

void FC_STAT_ASK_cb(const void *data, const prot_packet_info_t *info) {
  BIN_ASSERT(*(uint8_t *)data == FC_STAT_ASK);
  FC_STAT_s packet;
  packet.err_cnt = 1;
  packet.rx_cnt = 2;
  _BIN_Finalize(FC_STAT_RESP, &packet, packet.len, info);
}

const prot_cb_t prot_cb_tab[] = {
    {FC_SYNC_POLL, FC_SYNC_POLL_BIN_cb},
    {FC_CARRY, 0},
    {FC_STAT_ASK, FC_STAT_ASK_cb},
};
const int prot_cb_len = sizeof(prot_cb_tab) / sizeof(*prot_cb_tab);
