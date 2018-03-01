#ifndef _MAC_PORT_H
#define _MAC_PORT_H

#include "iassert.h"

#define MAC_ASSERT(expr) IASSERT(expr)

#define MAC_LOG_ERR(msg) LOG_ERR(msg)

typedef unsigned int mac_time_t;

static inline mac_time_t mac_port_get_time()
{
    return HAL_GetTick();
}

#endif // _MAC_PORT_H
