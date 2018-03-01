#ifndef _MAC_SETTINGS
#define _MAC_SETTINGS

#include "prot/prot_const.h" // ADDR_BROADCAST

#define MAC_BUF_CNT 2
#define MAC_BUF_LEN 128

typedef struct
{
    dev_addr_t addr;
    int slot_time;
    int slot_guard_time;
    int slot_number;
    int slots_sum_time;
    int max_frame_fail_cnt;
} mac_settings_t;

#define MAC_SETTINGS_DEF         \
    {                            \
        .addr = ADDR_BROADCAST,  \
        .slot_time = 123,        \
        .slot_guard_time = 10,   \
        .slot_number = 15,       \
        .slots_sum_time = 12345, \
        .max_frame_fail_cnt = 3, \
    }

#endif