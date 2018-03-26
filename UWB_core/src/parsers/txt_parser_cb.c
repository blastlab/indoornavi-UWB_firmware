#include "txt_parser.h"

void _TXT_Finalize(mac_buf_t *buf, const prot_packet_info_t *info) {
  if (info->direct_src == ADDR_BROADCAST) {
    buf->dPtr = &buf->buf[0];
    BIN_ParseSingle(buf, info);
    MAC_Free(buf);
  } else {
    MAC_Send(buf, true);
  }
}

void _TXT_Ask(const prot_packet_info_t *info, uint8_t FC) {
  dev_addr_t addr = info->direct_src;
  mac_buf_t *buf =
      addr == ADDR_BROADCAST ? MAC_Buffer() : CARRY_PrepareBufTo(addr);
  if (buf != 0) {
    MAC_Write8(buf, FC);
    MAC_Write8(buf, 2); // len
    _TXT_Finalize(buf, info);
  }
}

// === callbacks ===

void TXT_StatCb(const txt_buf_t *buf, const prot_packet_info_t *info) {
  _TXT_Ask(info, FC_STAT_ASK);
}

void TXT_VersionCb(const txt_buf_t *buf, const prot_packet_info_t *info) {
  _TXT_Ask(info, FC_VERSION_ASK);
}

void TXT_HangCb(const txt_buf_t *buf, const prot_packet_info_t *info) {
  while (1) {
  }
}

void TXT_TestCb(const txt_buf_t *buf, const prot_packet_info_t *info) {
  LOG_TEST("PASS");
}

const txt_cb_t txt_cb_tab[] = {{"stat", TXT_StatCb},
                               {"version", TXT_VersionCb},
                               {"_hang", TXT_HangCb},
                               {"test", TXT_TestCb}};

const int txt_cb_len = sizeof(txt_cb_tab) / sizeof(*txt_cb_tab);
