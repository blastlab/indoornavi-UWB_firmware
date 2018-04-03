#ifndef _MAC_SETTINGS
#define _MAC_SETTINGS

#include "mac_const.h" // ADDR_BROADCAST
#include "mac_port.h"  // mac_buff_time_t

#define MAC_BUF_CNT 2
#define MAC_BUF_LEN 128

#define TOA_MAX_DEV_IN_POLL 4
#define SYNC_MAC_NEIGHTBOURS 5

typedef enum {
  RTLS_TAG = 'T',
  RTLS_ANCHOR = 'A',
  RTLS_SINK = 'S',
  RTLS_LISTENER = 'L',
} rtls_role;

typedef struct {
  int fin_dly;
  int resp_dly[TOA_MAX_DEV_IN_POLL];
  int guard_time;
} toa_settings_t;

typedef struct {
  dev_addr_t addr;
  pan_dev_addr_t pan;
  int slot_time;
  int slot_guard_time;
  int slot_number;
  int slots_sum_time;
  int max_frame_fail_cnt;
  mac_buff_time_t max_buf_inactive_time;
  toa_settings_t sync_dly;
  rtls_role role;
  bool raport_anchor_anchor_distance;
} mac_settings_t;

#define _DEF_SLOT_TIME 123
#define _DEF_SLON_CNT 16
#define _DEF_SLOT_GUARD_TIME 2

#define _DEF_SLOT_FULL_TIME (_DEF_SLOT_TIME + _DEF_SLOT_GUARD_TIME)
#define _DEF_SLOT_SUM_TIME (_DEF_SLON_CNT * _DEF_SLOT_FULL_TIME)

#define MAC_SETTINGS_DEF                                                       \
  {                                                                            \
    .addr = ADDR_BROADCAST, .pan = 0xDECA, .slot_time = _DEF_SLOT_TIME,        \
    .slot_guard_time = _DEF_SLOT_GUARD_TIME, .slot_number = _DEF_SLON_CNT,     \
    .slots_sum_time = _DEF_SLOT_SUM_TIME, .max_frame_fail_cnt = 3,             \
    .max_buf_inactive_time = 2 * _DEF_SLOT_SUM_TIME, .role = RTLS_TAG,         \
    .raport_anchor_anchor_distance = true,                                     \
  }

#endif
