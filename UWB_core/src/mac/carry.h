#ifndef _CARRY_H
#define _CARRY_H

#include <string.h> // memcpy

#include "../mac/mac.h"
#include "../settings.h"

#include "carry_const.h"
#include "carry_settings.h"

#include "../parsers/bin_parser.h"

// protocol struct
typedef struct __packed
{
    unsigned char flag_hops;
    dev_addr_t src_addr;
    dev_addr_t hops[0]; // destination .. next hop
} FC_CARRY_s;

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
    bool isConnectedToServer;
    dev_addr_t toSinkId;
} carry_instance_t;

// initialize module data
void CARRY_Init(bool isConnectedToServer);

// write trace to target, including target address
// target address is the last one
// return number of written addresses or 0 when target is unknown
int CARRY_WriteTrace(dev_addr_t *buf, dev_addr_t target);

// reserve buffer, write message headers and set buffer fields
// to its default values. Field carry_flags can be one off CARRY_FLAG_xx,
// when tharget is different than
// When some error occure then return 0
mac_buf_t *CARRY_PrepareBufTo(dev_addr_t target);

// find or create buffer to the target device
mac_buf_t *CARRY_GetBufTo(dev_addr_t target);

// prepare response to the given devive
mac_buf_t *carry_prepare_response(const prot_packet_info_t *info);

// function called from MAC module after receiving frame
void CARRY_ParseMessage(buf);

#endif // _CARRY_H
