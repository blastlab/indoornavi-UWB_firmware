#include "ascii_parser.h"

void _ascii_finalize(mac_buf_t *buf, const prot_packet_info_t *info)
{
    if(info->direct_src == ADDR_BROADCAST)
    {
        buf->dPtr = &buf->buf[0];
        bin_parse(buf, info);
        mac_free();
    }
    else
    {
        carry_send(buf, true);
    }
}

void _ascii_ask(const prot_packet_info_t *info, uint8_t FC)
{
    dev_addr_t addr = info->direct_src;
    mac_buf_t *buf = addr == ADDR_BROADCAST ? mac_buffer() : carry_prepare_buf_to(addr);
    if(buf != 0)
    {
        mac_write8(buf, FC);
        mac_write8(buf, 2); // len
        _ascii_finalize(buf, info);
    }
}

// === callbacks ===

void ascii_stat_cb(const ascii_buf_t *buf, cchar *cmd, const prot_packet_info_t *info)
{
    _ascii_ask(info, FC_STAT_ASK);
}

void ascii_version_cb(const ascii_buf_t *buf, cchar *cmd, const prot_packet_info_t *info)
{
    _ascii_ask(info, FC_VERSION_ASK);
}

const ascii_cb_t ascii_cb_tab[] = {
    {"stat", ascii_stat_cb},
    {"version", ascii_version_cb}};

const int ascii_cb_len = sizeof(ascii_cb_tab) / sizeof(*ascii_cb_tab);
