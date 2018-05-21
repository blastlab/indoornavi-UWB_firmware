#ifndef _SYNC_H
#define _SYNC_H

#include <math.h>
#include <string.h>

#include "../iassert.h"
#include "../settings.h"
#include "../transceiver.h"
#include "mac_const.h"
#include "toa.h"

#define SYNC_ASSERT(expr) IASSERT(expr)

#define SYNC_TRACE_ENABLED 1
#if SYNC_TRACE_ENABLED
#define SYNC_TRACE(...) LOG_DBG(__VA_ARGS__)
#else
#include "tools.h"
#define SYNC_TRACE(...) ALL_UNUSED(__VA_ARGS__)
#endif

#define SYNC_TRACE_TOA_ENABLED 0
#if SYNC_TRACE_TOA_ENABLED
#define SYNC_TRACE_TOA(...) LOG_DBG(__VA_ARGS__)
#else
#include "tools.h"
#define SYNC_TRACE_TOA(...) ALL_UNUSED(__VA_ARGS__)
#endif

typedef struct __packed {
  uint8_t FC;
  uint8_t len;
  uint8_t num_poll_anchor;
  dev_addr_t poll_addr[0];
} FC_SYNC_POLL_s;

typedef struct __packed {
  uint8_t FC;
  uint8_t len;
  uint32_t TsPollRx;
  uint32_t TsRespTx;
} FC_SYNC_RESP_s;

typedef struct __packed {
  uint8_t FC;
  uint8_t len;
  uint8_t tree_level;
  uint8_t slot_num;
  uint32_t TsPollTx;
  uint8_t TsFinTxBuf[5];
  uint8_t TsOffset[5];
  uint32_t TsRespRx[0];
} FC_SYNC_FIN_s;

typedef struct {
  dev_addr_t addr;
  uint8_t tree_level;
  int64_t time_offset, update_ts;
  int64_t drift[3];
  float time_coeffP[3], time_coeffP_raw[3];
  float tof_dw;
} sync_neightbour_t;

typedef struct {
  toa_core_t toa;
  int64_t toa_ts_poll_rx_raw;
  uint8_t tree_level;
  sync_neightbour_t local_obj;
  sync_neightbour_t neightbour[5];
} sync_instance_t;

// Initialize Sync module
void SYNC_Init();

// Search for neightbour with a given addr. When such device isn't in table
// then initialize new if there is enouth memory for it
sync_neightbour_t* SYNC_FindOrCreateNeightbour(dev_addr_t addr, int tree_level);

// Send Poll message to start SYNC process with a given anchor
int SYNC_SendPoll(dev_addr_t dst, dev_addr_t anchors[], int anc_cnt);

int SYNC_RxCb(const void *data, const prot_packet_info_t *info);
int SYNC_RxToCb();
int SYNC_TxCb(int64_t TsDwTx);
#endif
