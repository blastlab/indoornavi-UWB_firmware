#ifndef _MAC_SETTINGS
#define _MAC_SETTINGS

#define MAC_BUF_CNT 2
#define MAC_BUF_LEN 128

#define mac_port_get_sync_time() GET_CLOCK()

typedef struct
{
    addr_t addr;
    int slot_time;
    int slot_guard_time;
    int slot_number;
    int slots_sum_time;
} mac_settings_t;

#define MAC_SETTINGS_DEF        \
    {                           \
        .addr = ADDR_BROADCAST, \
        .slot_time = 123,       \
        .slot_guard_time = 10,  \
        .slot_number = 15,      \
        .slots_sum_time = 12345 \
    }

#endif