#ifndef _CARRY_H
#define _CARRY_H

#include <string.h> // memcpy

#include "prot_const.h"
#include "mac/mac.h"
#include "settings.h"

#include "carry_settings.h"
#include "carry_const.h"

// protocol struct
typedef struct __packed
{
    unsigned char FC;
    unsigned char len;
    dev_addr_t src_addr;
    unsigned char flag_hops;
    dev_addr_t hops[0];
} carry_packet_t;

#define CARRY_HEAD_MIN_LEN (1 + 1 + sizeof(dev_addr_t) + 1)

// save data about some trace to target
typedef struct
{
    dev_addr_t path[CARRY_MAX_HOPS];
    int path_len;
    int pass_cnt;
    int fail_cnt;
    int cost;
    mac_buff_time_t last_update_time;
} carry_trace_t;

// save data about target
typedef struct
{
    dev_addr_t addr;
    carry_trace_t trace[CARRY_MAX_TRACE];
    mac_buff_time_t last_update_time;
} carry_target_t;

// global singleton
typedef struct
{
    carry_target_t target[CARRY_MAX_TARGETS];
} carry_instance_t;

// initialize module data
void carry_init();

// write trace to target, including target address
// target address is the last one
// return number of written addresses or 0 when target is unknown
int carry_write_trace(dev_addr_t *buf, dev_addr_t target);

// reserve buffer, write message headers and set buffer fields
// to its default values. Field carry_flags can be one off CARRY_FLAG_xx,
// when tharget is different than
// When some error occure then return 0
void carry_prepare(mac_buf_t *buf, dev_addr_t target, unsigned char carry_flags);

// add packet to the transmit queue
void carry_send(mac_buf_t *packet, bool require_ack);

// events handlers:

// handle incoming message
extern prot_parser_cb carry_parser;

#endif // _CARRY_H
