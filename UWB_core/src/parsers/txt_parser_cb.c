#include "txt_parser.h"

void _txt_finalize(mac_buf_t *buf, const prot_packet_info_t *info)
{
    if (info->direct_src == ADDR_BROADCAST)
    {
        buf->dPtr = &buf->buf[0];
        bin_parse(buf, info);
        mac_free(buf);
    }
    else
    {
        carry_send(buf, true);
    }
}

void _txt_ask(const prot_packet_info_t *info, uint8_t FC)
{
    dev_addr_t addr = info->direct_src;
    mac_buf_t *buf = addr == ADDR_BROADCAST ? mac_buffer() : carry_prepare_buf_to(addr);
    if (buf != 0)
    {
        mac_write8(buf, FC);
        mac_write8(buf, 2); // len
        _txt_finalize(buf, info);
    }
}

// === callbacks ===

void txt_stat_cb(const txt_buf_t *buf, cchar *cmd, const prot_packet_info_t *info)
{
    _txt_ask(info, FC_STAT_ASK);
}

void txt_version_cb(const txt_buf_t *buf, cchar *cmd, const prot_packet_info_t *info)
{
    _txt_ask(info, FC_VERSION_ASK);
}

void txt_hang_cb(const txt_buf_t *buf, cchar *cmd, const prot_packet_info_t *info)
{
    while (1)
    {
    }
}

void txt_test_cb(const txt_buf_t *buf, cchar *cmd, const prot_packet_info_t *info)
{
    LOG_TEST("PASS");
}

const txt_cb_t txt_cb_tab[] = {
    {"stat", txt_stat_cb},
    {"version", txt_version_cb},
    {"_hang", txt_hang_cb},
    {"test", txt_test_cb}};

const int txt_cb_len = sizeof(txt_cb_tab) / sizeof(*txt_cb_tab);
