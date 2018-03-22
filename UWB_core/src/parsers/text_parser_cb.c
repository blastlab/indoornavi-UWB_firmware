#include "text_parser.h"

void _text_finalize(const void *data, const prot_packet_info_t *info)
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

void _text_ask(const prot_packet_info_t *info, uint8_t FC)
{
    dev_addr_t addr = info->direct_src;
    mac_buf_t *buf = addr == ADDR_BROADCAST ? mac_buffer() : carry_prepare_buf_to(addr);
    if(buf != 0)
    {
        mac_write8(buf, FC);
        mac_write8(buf, 2); // len
        _text_finalize(buf, info);
    }
}

// === callbacks ===

void text_stat_cb(const text_buf_t *buf, cchar *cmd, const prot_packet_info_t *info)
{
    _text_ask(info, FC_STAT_ASK);
}

void text_version_cb(const text_buf_t *buf, cchar *cmd, const prot_packet_info_t *info)
{
    _text_ask(info, FC_VERSION_ASK);
}

const text_cb_t text_cb_tab[] = {
    {"stat", text_stat_cb},
    {"version", text_version_cb}};

const int text_cb_len = sizeof(text_cb_tab) / sizeof(*text_cb_tab);