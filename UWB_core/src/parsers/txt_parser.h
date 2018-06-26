#ifndef _TXT_PARSER_H
#define _TXT_PARSER_H

#include <ctype.h> // tolower

#include "tools.h"
#include "../mac/carry.h"
#include "bin_const.h"
#include "bin_struct.h"

typedef const char cchar;

typedef struct {
  cchar *cmd;
  cchar *const start;
  cchar *const end;
  int cnt;
} txt_buf_t;

typedef void (*txt_parser_cb)(const txt_buf_t *buf,
                              const prot_packet_info_t *info);

typedef struct {
  const char *cmd;
  const txt_parser_cb cb;
} txt_cb_t;

// return pointer to parameter number @num
cchar *TXT_PointParamNumber(const txt_buf_t *buf, cchar *cmd, int num);

// return integer from string (in circular buffer) pointed by ptr
int TXT_AtoI(const txt_buf_t *buf, cchar *ptr, int base);

// return pointer to
int TXT_GetParam(const txt_buf_t *buf, cchar *cmd, int base);

// return true if @buf.cmd starts with @cmd
bool TXT_StartsWith(const txt_buf_t *buf, cchar *cmd);

// parse command
void TXT_Parse(const txt_buf_t *buf);

// take input to data parser, ignore \r and split by \n
void TXT_Input(const char *str, int len);

// look for any new text message to parse
void TXT_Control();
#endif // _TXT_PARSER_H
