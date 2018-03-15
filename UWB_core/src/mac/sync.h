#ifndef _SYNC_H
#define _SYNC_H

#include <string.h>

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
    uint8_t TsPollTxBuf[5];
    uint32_t TsFinTx;
    uint32_t TsRespRx[MAC_SYNC_MAX_AN_CNT];
} FC_SYNC_FIN_s;

typedef enum {
    TOA_IDLE,
    TOA_POLL_WAIT_TO_SEND,
    TOA_POLL_SENT,
    TOA_RESP_REC,
    TOA_FIN_WAIT_TO_SEND,
    TOA_FIN_SENT,
    TOA_POLL_REC,
    TOA_RESP_WAIT_TO_SEND,
    TOA_RESP_SENT,
    TOA_FIN_REC,
} toa_state_t;

typedef struct
{
    toa_state_t state;
    uint64_t TsPollTx;
    uint32_t TsPollRx;
    uint32_t TsRespTx, TsRespRx[MAC_SYNC_MAX_AN_CNT];
    uint32_t TsFinTx, TsFinRx;
    uint8_t resp_ind;        // 0..anc_in_poll_cnt-1
    uint8_t anc_in_poll_cnt; // 1..MAC_SYNC_MAX_AN_CNT
    dev_addr_t addr_tab[MAC_SYNC_MAX_AN_CNT];
    dev_addr_t initiator;
} toa_core_t;

typedef struct
{
    dev_addr_t addr;
    uint8_t tree_level;
    uint64_t time_offset[3], update_ts;
    float time_coeffP[3];
} sync_neightbour_t;

typedef struct
{
    toa_core_t toa;
    uint64_t toa_ts_poll_rx_raw;
    uint8_t tree_level;
    sync_neightbour_t local_obj;
    sync_neightbour_t neightbour[5];
} sync_instance_t;

// set delayed tx time and return tx time in dw time units
uint64_t sync_set_tx_time(uint64_t dw_time, uint32_t delay_us);

// usefull when writing full timestamp
void sync_write_40b_value(uint8_t *dst, uint64_t val);

// usefull when reading full timestamp
uint64_t sync_read_40b_value(uint8_t *src);

int FC_SYNC_POLL_cb(const void *data, const prot_packet_info_t *info);
int FC_SYNC_RESP_cb(const void *data, const prot_packet_info_t *info);
int FC_SYNC_FIN_cb(const void *data, const prot_packet_info_t *info);

#define SYNC_CALLBACKS   \
    FC_SYNC_POLL_cb,     \
        FC_SYNC_RESP_cb, \
        FC_SYNC_FIN_cb,

#endif