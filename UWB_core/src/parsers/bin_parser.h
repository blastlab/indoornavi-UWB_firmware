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

extern const prot_cb_t prot_cb_tab[];;
extern const int prot_cb_len;

void bin_parse();

#endif // _PROT_PARSER_H