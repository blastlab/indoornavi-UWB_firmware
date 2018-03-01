#ifndef _TRANSCEIVER_SETTINGS_H
#define _TRANSCEIVER_SETTINGS_H

#include "iassert.h"
#define TRANSCEIVER_ASSERT(expr) IASSERT(expr)

typedef struct
{
    unsigned short ant_dly_rx, ant_dly_tx;
} transceiver_settings_t;

#define TRANSCEIVER_SETTINGS_DEF \
    {                            \
        .ant_dly_rx = 1,         \
        .ant_dly_tx = 2          \
    }

#endif