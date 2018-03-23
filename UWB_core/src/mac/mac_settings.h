#ifndef _MAC_SETTINGS
#define _MAC_SETTINGS

#include "../prot/prot_const.h" // ADDR_BROADCAST

#define MAC_BUF_CNT 2
#define MAC_BUF_LEN 128

#define TOA_MAX_DEV_IN_POLL 4
#define SYNC_MAC_NEIGHTBOURS 5

typedef struct
{
    int fin_dly;
    int resp_dly[TOA_MAX_DEV_IN_POLL];
    int guard_time;
} toa_settings_t;

typedef struct
{
    dev_addr_t addr;
    pan_dev_addr_t pan;
    int slot_time;
    int slot_guard_time;
    int slot_number;
    int slots_sum_time;
    int max_frame_fail_cnt;
    toa_settings_t sync_dly;
    bool raport_anchor_anchor_distance;
} mac_settings_t;

#define MAC_SETTINGS_DEF         \
    {                            \
        .addr = ADDR_BROADCAST,  \
        .pan = 0xDECA,           \
        .slot_time = 123,        \
        .slot_guard_time = 10,   \
        .slot_number = 15,       \
        .slots_sum_time = 12345, \
        .max_frame_fail_cnt = 3, \
        .raport_anchor_anchor_distance = true, \
    }

#endif