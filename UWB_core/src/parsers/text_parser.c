#include "text_parser.h"

char _text_buf_t[256];
text_buf_t text_buf = {
    .cmd = _text_buf_t,
    .start = _text_buf_t, 
    .end = _text_buf_t + sizeof(_text_buf_t)
};

// zwroc wskaznik za num spacjami albo 0
cchar * text_point_param_number(text_buf_t *buf, cchar *cmd, int num)
{
    cchar * ptr = cmd;
    while(num > 0)
    {
        while(*ptr != ' ' && *ptr != 0)
        {
            ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
        }
        while(*ptr == ' ')
        {
            ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
        }
        if(*ptr == 0 )
        {
            return 0;
        }
    }
    return ptr;
}

// return 
int text_atoi(text_buf_t *buf, cchar *cmd, int base)
{
    cchar *ptr = text_point_param_number(buf, cmd, num);
    int result = 0;
    if(ptr == 0)
    {
        return -1;
    }
    while(('0' <= *ptr && *ptr <= '9') ||
        (base == 16 && 'a' <= lower(*ptr) && lower(*ptr) <= 'f'))
    {
        result *= base;
        result += *ptr <= '9' ? *ptr - '0' : lower(*ptr) - 'a';
        ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
    }
    return result;
}

// return pointer to 
int text_get_param(text_buf_t *buf, cchar *cmd, int base)
{
    int i;
    cchar *ptr = buf->cmd;
    
    while(*ptr != 0)
    {
        for(i = 0; *ptr == cmd[i]; ++i)
        {
            ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
        }
        if(cmd[i] == 0)
        {
            return text_atoi(buf, ptr, base);
        }
        ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
    }
    return -1;
}

bool text_starts_with(text_buf_t *buf, cchar *cmd)
{
    cchar *ptr = buf->cmd;
    while(*cmd != 0)
    {
        if(*cmd != *ptr)
        {
            return false;
        }
        ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
        ++cmd;
    }
    return true;
}

void text_parse(const text_buf_t *buf)
{
    // in text_parser_cb.c
    extern const text_cb_t text_cb_tab[];
    extern const int text_cb_len;

    prot_packet_info_t info;
    memset(&info, 0, sizeof(info));
    int did = text_get_param(buf, "did:", 16);
    info.direct_src = did > 0 ? did : ADDR_BROADCAST;

    for(int i = 0; i < text_cb_len; ++i)
    {
        if(text_starts_with(buf, text_cb_tab[i].cmd))
        {
            IASSERT(text_cb_tab[i].cb != 0);
            text_cb_tab[i].cb(buf, &info);
            return;       
        }
    }
    LOG_ERR("Bad command (version, stat)");
}