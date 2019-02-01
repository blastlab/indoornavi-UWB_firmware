/**
 * @brief
 *
 * @file toa_routine.h
 * @author Karol Trzcinski
 * @date 2018-07-20
 */
#ifndef _TOA_ROUTINES_H
#define _TOA_ROUTINES_H

#include "../iassert.h"
#include "../settings.h"
#include "../transceiver.h"
#include "mac_const.h"
#include "toa.h"

typedef struct {
	uint8_t FC;
	uint8_t len;
	uint8_t padding;
	uint8_t num_poll_anchor;  ///< number of addresses in array poll_addr
	dev_addr_t poll_addr[0];  ///< list of anchors addresses to poll
}__packed FC_TOA_INIT_s;

typedef struct {
	uint8_t FC;
	uint8_t len;
	uint8_t padding;
	uint8_t num_poll_anchor;  ///< number of addresses in array poll_addr
	dev_addr_t poll_addr[0];  ///< list of anchors addresses to poll
}__packed FC_TOA_POLL_s;

typedef struct {
	uint8_t FC;
	uint8_t len;
	uint32_t TsPollRx;  ///< poll receive timestamp in dw time units
	uint32_t TsRespTx;  ///< response transmit timestamp in dw time units
}__packed FC_TOA_RESP_s;

typedef struct {
	uint8_t FC;
	uint8_t len;
	uint8_t slot_num;       ///< device mac slot number
	uint8_t TsFinTxBuf[5];  ///< 40b fin transmit timestamp in dw time units
	uint32_t TsPollTx;      ///< poll transmit timestamp in dw time units
	uint32_t TsRespRx[0];   ///< list of response receive timestamps in dtu
}__packed FC_TOA_FIN_s;

typedef struct {
	uint8_t FC;
	uint8_t len;
	measure_t meas;
}__packed FC_TOA_RES_s;

/**
 * @brief Initialize TOA module
 *
 */
void TOA_InitDly();

/**
 * @brief Send initiation message to start ranging from remote device
 *
 * Remote device after receiving this message should send Poll message
 * and start ranging procedure
 *
 * When target is a ANCHOR or anchors[0] is your address then send message directly to dst.
 * Otherwise destination address will be anchors[0]
 *
 * CARRY protocol is used as a transport layer
 *
 * @param[in] dst destination device address (tag)
 * @param[in] anchors ist of anchor to measure
 * @param[in] anc_cnt counter of anchors to measure
 * @return int 0 if succeed error code otherwise
 */
int TOA_SendInit(dev_addr_t dst, const dev_addr_t anchors[], int anc_cnt);


/**
 * @brief Send Poll message to start SYNC process with a given anchor
 *
 * @param[in] anchors ist of anchor to measure
 * @param[in] anc_cnt counter of anchors to measure
 * @return int 0 if succeed error code otherwise
 */
int TOA_SendPoll(const dev_addr_t anchors[], int anc_cnt);

/**
 * @brief send ranging result to your sink
 *
 * CARRY protocol is used as a transport layer
 *
 * @param[in] measure is a pointer to ranging data to send
 */
int TOA_SendRes(const measure_t* measure);

/**
 * @brief Init message callback routine
 *
 * This function should be added to binary parser callbacks array
 *
 * @param[in] data pointer to data to parse
 * @param[in] info extra packet info
 */
void FC_TOA_INIT_cb(const void* data, const prot_packet_info_t* info);

void FC_TOA_RES_cb(const void* data, const prot_packet_info_t* info);

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

#endif
