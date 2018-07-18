/**
 * @brief carry settings typedef and default values
 * 
 * @file carry_settings.h
 * @author Karol Trzcinski
 * @date 2018-07-02
 */
#ifndef _CARRY_SETTINGS_H
#define _CARRY_SETTINGS_H

#include "../mac/mac_port.h" // mac_buff_time_t

#define CARRY_ASSERT(expr) IASSERT(expr)

/**
 * @brief maximum number of parents for one anchor
 * 
 */
#define CARRY_MAX_TRACE 2

/**
 * @brief number of anchors traces in sink device
 * @deprecated change structure to anchor->parent
 * 
 */
#define CARRY_MAX_TARGETS 8

/**
 * @brief carry settings typedef
 * 
 */
typedef struct
{
  mac_buff_time_t trace_max_inactive_time; ///< max time from last message to be kept
  int trace_max_fail_cnt;  ///< trace retransmit/delete threshold
} carry_settings_t;

#define CARRY_SETTINGS_DEF                           \
  {                                                  \
    .trace_max_inactive_time = 1000, .trace_max_fail_cnt = 5 \
  \
}

#endif
