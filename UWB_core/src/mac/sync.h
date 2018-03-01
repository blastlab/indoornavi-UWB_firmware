#ifndef _SYNC_H
#define _SYNC_H

#include "iassert.h"
#include "prot/prot_const.h"
#include "settings.h"
#include "transceiver.h"

#define SYNC_ASSERT(expr) IASSERT(expr)

typedef struct __packed
{
    uint8_t FC;
    uint8_t len;
    uint8_t slot_num;
    uint8_t tree_level;
    uint64_t TsPollTx;
    uint8_t num_poll_anchor;
    dev_addr_t poll_addr[0];
} FC_SYNC_POLL_s;

typedef struct __packed
{
    uint8_t FC;
    uint8_t len;
    uint32_t TsPollRx;
    uint32_t TsRespTx;
} FC_SYNC_RESP_s;

typedef struct __packed
{
    uint8_t FC;
    uint8_t len;
    uint32_t TsFinTx;
    uint32_t TsRespRx[0];
} FC_SYNC_FIN_s;

typedef struct
{
    uint64_t TsPollTx;
    uint32_t TsPollRx;
    uint32_t TsRespTx, TsRespRx[4];
    uint32_t TsFinTx, TsFinRx;
} toa_core_t;

typedef struct
{
    dev_addr_t addr;
    uint8_t tree_level;
    uint64_t time_offset, update_ts;
    float time_coeffP;
} sync_neightbour;

typedef struct
{
    toa_core_t toa;
    sync_neightbour neightbour[5];
} sync_instance_t;

// set delayed tx time and return tx time in dw time units
uint64_t sync_set_tx_time(uint64_t dw_time, uint32_t delay_us);

int FC_SYNC_POLL_cb(const void *data, const void *prot_packet_info_t);
int FC_SYNC_RESP_cb(const void *data, const void *prot_packet_info_t);
int FC_SYNC_FIN_cb(const void *data, const void *prot_packet_info_t);

#define SYNC_CALLBACKS   \
    FC_SYNC_POLL_cb,     \
        FC_SYNC_RESP_cb, \
        FC_SYNC_FIN_cb,

#endif