#include "txt_parser.h"

char _txt_buf_raw[256];
txt_buf_t txt_buf = {
    .cmd = _txt_buf_raw,
    .start = _txt_buf_raw,
    .end = _txt_buf_raw + sizeof(_txt_buf_raw)};

// zwroc wskaznik za num spacjami albo 0
cchar *txt_point_param_number(txt_buf_t *buf, cchar *cmd, int num)
{
    cchar *ptr = cmd;
    while (num > 0)
    {
        while (*ptr != ' ' && *ptr != 0)
        {
            ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
        }
        while (*ptr == ' ')
        {
            ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
        }
        if (*ptr == 0)
        {
            return 0;
        }
    }
    return ptr;
}

// return
int txt_atoi(txt_buf_t *buf, cchar *ptr, int base)
{
    int result = 0;
    if (ptr == 0)
    {
        return -1;
    }
    while (('0' <= *ptr && *ptr <= '9') ||
           (base == 16 && 'a' <= lower(*ptr) && lower(*ptr) <= 'f'))
    {
        result *= base;
        result += *ptr <= '9' ? *ptr - '0' : lower(*ptr) - 'a';
        ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
    }
    return result;
}

// return pointer to
int txt_get_param(txt_buf_t *buf, cchar *cmd, int base)
{
    int i;
    cchar *ptr = buf->cmd;

    while (*ptr != 0)
    {
        for (i = 0; *ptr == cmd[i]; ++i)
        {
            ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
        }
        if (cmd[i] == 0)
        {
            return txt_atoi(buf, ptr, base);
        }
        ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
    }
    return -1;
}

bool txt_starts_with(txt_buf_t *buf, cchar *cmd)
{
    cchar *ptr = buf->cmd;
    while (*cmd != 0)
    {
        if (*cmd != *ptr)
        {
            return false;
        }
        ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
        ++cmd;
    }
    return true;
}

void txt_parse(const txt_buf_t *buf)
{
    // in txt_parser_cb.c
    extern const txt_cb_t txt_cb_tab[];
    extern const int txt_cb_len;

    prot_packet_info_t info;
    memset(&info, 0, sizeof(info));
    int did = txt_get_param(buf, "did:", 16);
    info.direct_src = did > 0 ? did : ADDR_BROADCAST;

    for (int i = 0; i < txt_cb_len; ++i)
    {
        if (txt_starts_with(buf, txt_cb_tab[i].cmd))
        {
            //IASSERT(txt_cb_tab[i].cb != 0);
            txt_cb_tab[i].cb(buf, &info);
            return;
        }
    }
    LOG_ERR("Bad command (version, stat)");
}