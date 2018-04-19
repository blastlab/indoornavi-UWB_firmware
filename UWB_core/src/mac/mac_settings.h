#ifndef _MAC_SETTINGS
#define _MAC_SETTINGS

#include "mac_const.h" // ADDR_BROADCAST
#include "mac_port.h"  // mac_buff_time_t

#define MAC_BUF_CNT 5
#define MAC_BUF_LEN 128

#define TOA_MAX_DEV_IN_POLL 4
#define SYNC_MAC_NEIGHTBOURS 5

typedef enum {
  RTLS_TAG = 'T',
  RTLS_ANCHOR = 'A',
  RTLS_SINK = 'S',
  RTLS_LISTENER = 'L',
  RTLS_DEFAULT = 'D'
} rtls_role;

typedef struct {
  int fin_dly_us; // tx dly after last resp
  int resp_dly_us[TOA_MAX_DEV_IN_POLL];
  int guard_time_us;         // time margin during receive
  int rx_after_tx_offset_us; // time distance between rx on and rx anticipation
} toa_settings_t;

typedef struct {
  dev_addr_t addr;
  pan_dev_addr_t pan;
  int slot_time_us;
  int slot_guard_time_us;
  int slots_sum_time_us;
  int slot_number;
  int max_frame_fail_cnt;
  mac_buff_time_t max_buf_inactive_time;
  toa_settings_t sync_dly;
  rtls_role role;
  bool raport_anchor_anchor_distance;
} mac_settings_t;

#define _DEF_SLOT_TIME 10000
#define _DEF_SLON_CNT 16

#define _DEF_SLOT_SUM_TIME (_DEF_SLON_CNT * _DEF_SLOT_TIME)

#define MAC_SETTINGS_DEF                                                       \
  {                                                                            \
    .addr = ADDR_BROADCAST, .pan = 0xDECA, .slot_time_us = _DEF_SLOT_TIME,     \
    .slot_guard_time_us = 200, .slot_number = _DEF_SLON_CNT,                   \
    .slots_sum_time_us = _DEF_SLOT_SUM_TIME, .max_frame_fail_cnt = 3,          \
    .max_buf_inactive_time = 2 * _DEF_SLOT_SUM_TIME, .role = RTLS_DEFAULT,     \
    .raport_anchor_anchor_distance = true,                                     \
  }

#endif
