/**
 * @brief carry settings typedef and default values
 *
 * @file carry_settings.h
 * @author Karol Trzcinski
 * @date 2018-07-02
 */
#ifndef _CARRY_SETTINGS_H
#define _CARRY_SETTINGS_H

#include "../mac/mac_port.h"  // mac_buff_time_t

#define CARRY_ASSERT(expr) IASSERT(expr)

/**
 * @brief maximum number of parents for one anchor
 *
 */
#define CARRY_MAX_PARENTS 2

/**
 * @brief maximum number of remembered anchors
 *
 */
#define CARRY_MAX_TARGETS 64

/**
 * @brief maximum number of remembered tags
 * 
 */
#define CARRY_MAX_TAGS 128

/**
 * @brief target info struct
 *
 */
typedef struct {
	dev_addr_t addr;
	dev_addr_t parents[CARRY_MAX_PARENTS];
	int16_t parentsScore[CARRY_MAX_PARENTS];
	int16_t level;
	time_ms_t lastUpdateTime;
} carry_target_t;

/**
 * @brief carry settings typedef
 *
 */
typedef struct {
	time_ms_t traceMaxInactiveTime;  ///< max time from last message to be kept
	int traceMaxFailCnt;       ///< trace retransmit/delete threshold
	int targetCounter;
	bool autoRoute;
	int minParentLiveTimeMs;
	int tagMaxInactiveTime; ///< max time before deleting tag from memory
	carry_target_t target[CARRY_MAX_TARGETS];
} carry_settings_t;

#define CARRY_SETTINGS_DEF                                                  \
  {                                                                         \
    .traceMaxInactiveTime = 1000, .traceMaxFailCnt = 5, .targetCounter = 0, \
    .autoRoute = false, .minParentLiveTimeMs = 1000                         \
  }

#endif
