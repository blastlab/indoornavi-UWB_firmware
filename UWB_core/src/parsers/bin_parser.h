#ifndef _BIN_PARSER_H
#define _BIN_PARSER_H

#include "../iassert.h"
#include "../prot/prot_const.h"
#include "../mac/mac.h"

#define BIN_ASSERT(expr) IASSERT(expr)


typedef void (*prot_parser_cb)(const void *data, const prot_packet_info_t *info);

typedef struct
{
    uint8_t FC;
    prot_parser_cb cb;
} prot_cb_t;

extern const prot_cb_t prot_cb_tab[];
extern const int prot_cb_len;

uint8_t BIN_ParseSingle(mac_buf_t *buf, const prot_packet_info_t *info);
void BIN_Parse(mac_buf_t *buf, const prot_packet_info_t *info, uint8_t size);

#endif // _PROT_PARSER_H
