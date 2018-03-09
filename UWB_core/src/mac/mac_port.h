#ifndef _MAC_PORT_H
#define _MAC_PORT_H

#include "iassert.h"
#include "platform/port.h"

#define MAC_ASSERT(expr) IASSERT(expr)

#define MAC_LOG_ERR(msg) LOG_ERR(msg)

typedef unsigned int mac_buff_time_t;

#define mac_port_buff_time() port_tick_hr()

#endif // _MAC_PORT_H
