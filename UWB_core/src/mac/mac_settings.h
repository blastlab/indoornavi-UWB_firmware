#ifndef _MAC_SETTINGS
#define _MAC_SETTINGS

#include "mac_const.h" // ADDR_BROADCAST
#include "mac_port.h"  // mac_buff_time_t

#define MAC_BUF_CNT 5
#define MAC_BUF_LEN 128

#define TOA_MEASURES_BUF_SIZE 5
#define TOA_MAX_DEV_IN_POLL 4
#define SYNC_MAC_NEIGHBOURS 5

typedef enum
{
  RTLS_TAG = 'T',
  RTLS_ANCHOR = 'A',
  RTLS_SINK = 'S',
  RTLS_LISTENER = 'L',
  RTLS_DEFAULT = 'D'
} rtls_role;

typedef struct
{
  int fin_dly_us; // tx dly after last resp
  int resp_dly_us[TOA_MAX_DEV_IN_POLL];
  int guard_time_us;         // time margin during receive
  int rx_after_tx_offset_us; // time distance between rx on and rx anticipation
} toa_settings_t;

typedef struct
{
  dev_addr_t addr;  ///< local device address
  pan_dev_addr_t pan;  ///< personal area network
  int slot_time_us;  ///< one slot time in us (including guard time)
  int slot_guard_time_us;  ///< guard time between slots
  int slot_tolerance_time_us; ///< tolerance time for sending packets before your slot time
  int slots_sum_time_us;  ///< slots sum time in us
  int max_frame_fail_cnt;  ///< frame retransmit/delete threshold
  mac_buff_time_t max_buf_inactive_time;  ///< maximal buf inactive time
  toa_settings_t sync_dly;  ///< SYNC TOA delay settings
  toa_settings_t toa_dly;  ///< TOA delay settings
  rtls_role role;  ///< local device 
  bool raport_anchor_anchor_distance;  ///< true 
} mac_settings_t;

// default one slot period (icluding guard time) converted from ms to us
#define _DEF_SLOT_TIME 10 * 1000

// default number of slots
#define _DEF_SLOT_CNT 10

#define _DEF_SLOT_SUM_TIME (_DEF_SLOT_CNT * _DEF_SLOT_TIME)

#define MAC_SETTINGS_DEF                                                                                           \
  {                                                                                                                \
    .addr = ADDR_BROADCAST, .pan = 0xDECA, .slot_time_us = _DEF_SLOT_TIME,                                         \
    .slot_guard_time_us = 500, .slot_tolerance_time_us = 50, .slots_sum_time_us = _DEF_SLOT_SUM_TIME, .max_frame_fail_cnt = 3,                   \
    .max_buf_inactive_time = 2 * _DEF_SLOT_SUM_TIME, .role = RTLS_DEFAULT, .raport_anchor_anchor_distance = false, \
  }

#endif
