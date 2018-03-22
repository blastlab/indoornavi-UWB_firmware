#ifndef _ASCII_PARSER_H
#define _ASCII_PARSER_H

#include "bin_const.h"
#include "bin_struct.h"
#include "../prot/carry.h"

typedef const char cchar;

typedef struct{
    cchar * cmd;
    cchar * const start;
    cchar * const end;
} ascii_buf_t;

typedef void (*ascii_parser_cb)(const ascii_buf_t *buf, const prot_packet_info_t *info);

typedef struct
{
    const char *cmd;
    const ascii_parser_cb cb;
} ascii_cb_t;

extern ascii_buf_t ascii_buf;

// return pointer to parameter number @num
cchar * ascii_point_param_number(ascii_buf_t *buf, cchar *cmd, int num);

// return integer from string (in circular buffer) pointed by ptr
int ascii_atoi(ascii_buf_t *buf, cchar *ptr, int base);

// return pointer to 
int ascii_get_param(ascii_buf_t *buf, cchar *cmd, int base);

// return true if @buf.cmd starts with @cmd
bool ascii_starts_with(ascii_buf_t *buf, cchar *cmd);

// parse command
void ascii_parse(const ascii_buf_t *buf);

#endif
