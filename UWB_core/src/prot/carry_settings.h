#ifndef _CARRY_SETTINGS_H
#define _CARRY_SETTINGS_H

#include "../mac/mac_port.h" // mac_buff_time_t

#define CARRY_ASSERT(expr) IASSERT(expr)

#define CARRY_MAX_TRACE 2
#define CARRY_MAX_TARGETS 8

typedef struct
{
  mac_buff_time_t max_inactive_time;
  int max_fail_counter;
} carry_settings_t;

#define CARRY_SETTINGS_DEF                           \
  {                                                  \
    .max_inactive_time = 1000, .max_fail_counter = 5 \
  \
}

#endif
