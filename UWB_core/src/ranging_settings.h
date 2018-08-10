/**
 * @brief measure traces settings
 *
 * @file ranging_settings.h
 * @author Karol Trzcinski
 * @date 2018-08-08
 */

#ifndef _RANGING_SETTINGS_H
#define _RANGING_SETTINGS_H

#include "mac/mac_const.h"     // dev_addr_t
#include "mac/mac_settings.h"  // TOA_MAX_DEV_IN_POLL

#define MEASURE_TRACE_MEMORY_DEPTH 256
#define RANGING_TEMP_ANC_LIST_DEPTH 16

typedef struct {
  dev_addr_t tagDid;                       ///< measure initiator did
  dev_addr_t ancDid[TOA_MAX_DEV_IN_POLL];  ///< list of did anchors to measure
  uint8_t numberOfAnchors;                 ///< number of anchors in poll
  uint8_t failCnt;  ///< number of consecutively failed measures
} measure_init_info_t;

typedef struct {
  measure_init_info_t measure[MEASURE_TRACE_MEMORY_DEPTH];  ///< measures buf
  int measureCnt;                                           ///< measure counter
  int rangingPeriodMs;  ///< time for whole ranging cycle
  int rangingDelayMs;   ///< time from one ranging to another
} ranging_settings_t;

#define RANGING_SETTINGS_DEF \
  { .measureCnt = 0, .rangingPeriodMs = 500, .rangingDelayMs = 10 }

#endif
