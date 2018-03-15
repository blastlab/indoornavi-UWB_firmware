#ifndef _PROT_CONST
#define _PROT_CONST

#include <stdbool.h>
#include <stdint.h>

typedef unsigned short dev_addr_t;
typedef unsigned short pan_dev_addr_t;
#define ADDR_BROADCAST 0xffff

#ifndef CORTEX_M
#ifndef __packed
#define __packed
#endif
#endif

typedef struct
{
    dev_addr_t direct_src;
} prot_packet_info_t;

typedef int (*prot_parser_cb)(const void *data, const void *prot_packet_info_t);

#define FC_CARRY 0x01

//#define FC_SYNC_INIT 0x10
#define FC_SYNC_POLL 0x11
#define FC_SYNC_RESP 0x12
#define FC_SYNC_FIN 0x13

#endif