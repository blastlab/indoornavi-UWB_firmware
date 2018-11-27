#ifndef _MAC_PORT_H
#define _MAC_PORT_H

#include "../iassert.h"
#include "platform/port.h"

#define MAC_ASSERT(expr) IASSERT(expr)

#define MAC_LOG_ERR(msg) LOG_ERR(msg)

typedef unsigned int mac_buff_time_t;

static inline mac_buff_time_t mac_port_buff_time() {
	return PORT_TickHr();
}

#endif  // _MAC_PORT_H
