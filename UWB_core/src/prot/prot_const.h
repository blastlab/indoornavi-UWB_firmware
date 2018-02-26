#ifndef _PROT_CONST
#define _PROT_CONST

#include "stdbool.h"

typedef unsigned short addr_t;
#define ADDR_BROADCAST 0xffff

typedef struct
{
    addr_t direct_src;
} prot_packet_info_t;

typedef short (*prot_parser_cb)(const void *data, const void *prot_packet_info_t);

#endif