#ifndef _CARRY_SETTINGS_H
#define _CARRY_SETTINGS_H

#include "mac_port.h" // mac_time_t

#define CARRY_MAX_HOPS 8
#define CARRY_MAX_TRACE 2
#define CARRY_MAX_TARGETS 8

typedef struct
{
    mac_time_t max_inactive_time;
} carry_settings_t;

#define CARRY_SETTINGS_DEF        \
    {                             \
        .max_inactive_time = 1000 \
    }

#endif