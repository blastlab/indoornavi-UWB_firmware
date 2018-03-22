#ifndef _TEXT_PARSER_H
#define _TEXT_PARSER_H

#include "bin_const.h"
#include "bin_struct.h"
#include "../prot/carry.h"

typedef const char cchar;

typedef struct{
    cchar * cmd;
    cchar * const start;
    cchar * const end;
} text_buf_t;

typedef void (*text_parser_cb)(const text_buf_t *buf, const prot_packet_info_t *info);

typedef struct
{
    const char *cmd;
    const text_parser_cb cb;
} text_cb_t;

// return pointer to parameter number @num
cchar * text_point_param_number(text_buf_t *buf, cchar *cmd, int num);

// return integer from string (in circular buffer)
int text_atoi(text_buf_t *buf, cchar *cmd, int base);

// return pointer to 
int text_get_param(text_buf_t *buf, cchar *cmd, int base);

bool text_starts_with(text_buf_t *buf, cchar *cmd);
#endif