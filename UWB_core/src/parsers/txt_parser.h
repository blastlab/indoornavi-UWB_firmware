#ifndef _TXT_PARSER_H
#define _TXT_PARSER_H

#include "bin_const.h"
#include "bin_struct.h"
#include "../prot/carry.h"

typedef const char cchar;

typedef struct
{
    cchar *cmd;
    cchar *const start;
    cchar *const end;
} txt_buf_t;

extern txt_buf_t txt_buf;

typedef void (*txt_parser_cb)(const txt_buf_t *buf, const prot_packet_info_t *info);

typedef struct
{
    const txt_parser_cb cb;
    const char *cmd;
} txt_cb_t;

// return pointer to parameter number @num
cchar *txt_point_param_number(txt_buf_t *buf, cchar *cmd, int num);

// return integer from string (in circular buffer) pointed by ptr
int txt_atoi(txt_buf_t *buf, cchar *ptr, int base);

// return pointer to
int txt_get_param(txt_buf_t *buf, cchar *cmd, int base);

// return true if @buf.cmd starts with @cmd
bool txt_starts_with(txt_buf_t *buf, cchar *cmd);

// parse command
void txt_parse(const txt_buf_t *buf);

#endif // _TXT_PARSER_H
