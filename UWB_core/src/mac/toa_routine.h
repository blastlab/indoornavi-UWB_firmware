/**
 * @brief
 *
 * @file toa_routine.h
 * @author Karol Trzcinski
 * @date 2018-07-20
 */

#include "../iassert.h"
#include "../settings.h"
#include "../transceiver.h"
#include "mac_const.h"
#include "toa.h"

typedef struct __packed {
  uint8_t FC;
  uint8_t len;
  uint8_t padding;
  uint8_t num_poll_anchor;  ///< number of addresses in array poll_addr
  dev_addr_t poll_addr[0];  ///< list of anchors addresses to poll
} FC_TOA_INIT_s;

typedef struct __packed {
  uint8_t FC;
  uint8_t len;
  uint8_t padding;
  uint8_t num_poll_anchor;  ///< number of addresses in array poll_addr
  dev_addr_t poll_addr[0];  ///< list of anchors addresses to poll
} FC_TOA_POLL_s;

typedef struct __packed {
  uint8_t FC;
  uint8_t len;
  uint32_t TsPollRx;  ///< poll receive timestamp in dw time units
  uint32_t TsRespTx;  ///< response transmit timestamp in dw time units
} FC_TOA_RESP_s;

typedef struct __packed {
  uint8_t FC;
  uint8_t len;
  uint8_t slot_num;
  uint8_t TsFinTxBuf[5];  ///< 40b fin transmit timestamp in dw time units
  uint32_t TsPollTx;      ///< poll transmit timestamp in dw time units
  uint32_t TsRespRx[0];   ///< list of response receive timestamps in dtu
} FC_TOA_FIN_s;

typedef struct __packed {
  uint8_t FC;
  uint8_t len;
  measure_t meas;
} FC_TOA_RES_s;

/**
 * @brief Initialize TOA module
 *
 */
void TOA_InitDly();

/**
 * @brief Send Poll message to start SYNC process with a given anchor
 *
 * @param[in] dst destination device address
 * @param[in] anchors ist of anchor to measure ()
 * @param[in] anc_cnt counter of anchors to measure
 * @return int
 */
int TOA_SendPoll(dev_addr_t dst, dev_addr_t anchors[], int anc_cnt);

/**
 * @brief sync rx callback
 *
 * @param[in] data pointer to data to parse
 * @param[in] info extra packet info
 * @return int 1 if packed was parser 0 otherwise
 */
int TOA_RxCb(const void* data, const prot_packet_info_t* info);

/**
 * @brief sync rx timeout routine
 *
 * @return int 1 if packed was parsed 0 otherwise
 */
int TOA_RxToCb();

/**
 * @brief sync tx callback
 *
 * @param[in] TsDwTx transmit timestamp in local dtu
 * @return int 1 if packed was parsed 0 otherwise
 */
int TOA_TxCb(int64_t TsDwTx);
