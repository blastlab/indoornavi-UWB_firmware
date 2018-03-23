#ifndef _PROT_CONST
#define _PROT_CONST

#include <stdbool.h>
#include <stdint.h>
#include "../parsers/bin_const.h"

typedef unsigned short dev_addr_t;
typedef unsigned short pan_dev_addr_t;
#define ADDR_BROADCAST 0xffff

#ifndef CORTEX_M
#ifndef __packed
#define __packed
#endif
#endif

struct FC_CARRY_s;

typedef struct
{
    dev_addr_t direct_src;
    struct FC_CARRY_s *carry;
} prot_packet_info_t;
#endif
