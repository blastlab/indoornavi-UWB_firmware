/**
 * @brief Time synchronization module
 *
 * @file sync.h
 * @author Karol Trzcinski
 * @date 2018-07-02
 *
 * It is usefull for time synchronization for collision avoidance and
 * for tags localization. To achieve precise localization synchronization should
 * be invoked frequently.
 *
 */
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

/// set 1 to trace sync debug messages
#define SYNC_TRACE_ENABLED 0
#if SYNC_TRACE_ENABLED
#define SYNC_TRACE(...) LOG_DBG(__VA_ARGS__)
#else
#include "tools.h"
#define SYNC_TRACE(...) ALL_UNUSED(__VA_ARGS__)
#endif

/// set 1 to trace sync time logging
#define SYNC_TIME_DUMP_ENABLED 0
#if SYNC_TIME_DUMP_ENABLED
#define SYNC_TIME_DUMP(...) LOG_DBG(__VA_ARGS__)
#else
#include "tools.h"
#define SYNC_TIME_DUMP(...) ALL_UNUSED(__VA_ARGS__)
#endif

/// set 1 to trace sync time of arrival debug messages
#define SYNC_TRACE_TOA_ENABLED 0
#if SYNC_TRACE_TOA_ENABLED
#define SYNC_TRACE_TOA(...) LOG_DBG(__VA_ARGS__)
#else
#include "tools.h"
#define SYNC_TRACE_TOA(...) ALL_UNUSED(__VA_ARGS__)
#endif

typedef struct {
	uint8_t FC;
	uint8_t len;
	uint8_t num_poll_anchor;  ///< number of addresses in array poll_addr
	dev_addr_t poll_addr[0];  ///< list of anchors addresses to poll
}__packed FC_SYNC_POLL_s;

typedef struct {
	uint8_t FC;
	uint8_t len;
	uint32_t TsPollRx;  ///< poll receive timestamp in dw time units
	uint32_t TsRespTx;  ///< response transmit timestamp in dw time units
}__packed FC_SYNC_RESP_s;

typedef struct {
	uint8_t FC;
	uint8_t len;
	uint8_t tree_level;     ///< device level in tree
	uint8_t slot_num;       ///< device slot number
	uint32_t TsPollTx;      ///< poll transmit timestamp in dw time units
	uint8_t TsFinTxBuf[5];  ///< 40b fin transmit timestamp in dw time units
	uint8_t TsOffset[5];    ///< 40b device local to global time offset
	uint32_t TsRespRx[0];   ///< list of response receive timestamps in dtu
}__packed FC_SYNC_FIN_s;

/**
 * @brief see #FC_t description
 */
typedef struct {
	uint8_t FC, len;
	uint16_t batt_voltage; ///< battery voltage in millivolts
	uint16_t px, py, pz; ///< current device position vector in cm
	uint8_t orientation; ///< hi nibble is vertical orientation, low nibble is horizontal orientation
	uint8_t seq; ///< TDOA sent tag beacon counter (used as packet identifier)
	uint32_t serial_hi; ///< device serial number from settings.version.serial
	uint32_t serial_lo; ///< device serial number from settings.version.serial
}__packed FC_TDOA_BEACON_TAG_s;

/**
 * @brief see #FC_t description
 *
 * This struct is generated for FC_TDOA_BEACON_TAG_s and for FC_TDOA_BEACON_ANCHOR_s.
 */
typedef struct {
	uint8_t FC, len;
	uint16_t batt_voltage; ///< battery voltage in millivolts
	uint16_t px, py, pz; ///< current device position vector in cm
	uint8_t orientation; ///< hi nibble is vertical orientation, low nibble is horizontal orientation
	uint8_t seq; ///< TDOA sent tag beacon counter (used as packet identifier)
	uint32_t serial_hi; ///< device serial number from settings.version.serial
	uint32_t serial_lo; ///< device serial number from settings.version.serial
	dev_addr_t tag_addr; ///< tag address
	dev_addr_t anchor_addr; ///< anchor address
	uint8_t rx_ts_glo[5]; ///< beacon receive timestamp in global time units from anchor device
	uint8_t rx_ts_loc[5]; ///< *optional, beacon receive timestamp in local time units from anchor device
}__packed FC_TDOA_BEACON_TAG_INFO_s;


/**
 * @brief see #FC_t description
 */
typedef struct {
	uint8_t FC, len;
	uint8_t seq; ///< TDOA sent tag beacon counter (used as packet identifier)
	uint8_t padding; ///< dummy byte
	uint8_t ts_glo[5]; ///< global tx timestamp from device
	uint8_t ts_loc[5]; ///< local tx timestamp from device
}__packed FC_TDOA_BEACON_ANCHOR_s;

/**
 * @brief see #FC_t description
 *
 * This struct is generated for FC_TDOA_BEACON_TAG_s and for FC_TDOA_BEACON_ANCHOR_s.
 */
typedef struct {
	uint8_t FC, len;
	dev_addr_t anchor_tx_addr; ///< anchor1 address
	dev_addr_t anchor_rx_addr; ///< anchor2 address
	uint8_t tof;					///< time of flight in dwt time units or zero
	uint8_t seq; ///< TDOA sent anchor beacon counter (used as packet identifier)
	uint8_t tx_ts_glo[5]; ///< beacon receive timestamp in global time units from anchor device
	uint8_t rx_ts_glo[5]; ///< beacon receive timestamp in global time units from anchor device
	uint8_t tx_ts_loc[5]; ///< *optional, beacon receive timestamp in local time units from anchor device
	uint8_t rx_ts_loc[5]; ///< *optional, beacon receive timestamp in local time units from anchor device
}__packed FC_TDOA_BEACON_ANCHOR_INFO_s;

typedef struct {
	dev_addr_t addr;           ///< neigtbour address
	uint8_t tree_level;        ///< number of hops from sink (sink is level 0)
	uint8_t sync_ready;        ///< sync ready (true or false)
	int64_t time_offset;       ///< local to global time offset in dtu
	int64_t update_ts;         ///< last update timestamp in dtu
	int64_t drift[3];          ///< list of last clock drifts
	float time_coeffP[3];      ///< list of last time drifts over delta dtu after
	float time_coeffP_raw[3];  ///< raw list of last time drifts over delta dtu
	float time_drift_sum;      ///< sum of clock drifts
	float tof_dw;  ///< time of flight between neighbour and local device in dtu
} sync_neighbour_t;

typedef struct {
	toa_core_t toa;              ///< SYNC toa data
	int64_t toa_ts_poll_rx_raw;  ///< used to SYNC #TOA_EnableRxBeforeFin
	bool sleep_after_tx;         ///< sync beacon sending flag
	uint8_t tree_level;          ///< local device tree level
	sync_neighbour_t local_obj;  ///< local clock from dwt data
	sync_neighbour_t neighbour[SYNC_MAC_NEIGHBOURS];  ///< list of neighbours
} sync_instance_t;

/**
 * @brief Initialize Sync module
 *
 */
void SYNC_Init();

/**
 * @brief return global time
 *
 * @param[in] dw_ts local dwt time
 * @return int64_t global dwt time
 */
int64_t SYNC_GlobTime(int64_t dw_ts);

/**
 * @brief Search for neighbour with a given addr
 *
 * When such device isn't in table then initialize new if there is enough memory
 * for it.
 *
 * @param[in] addr neighbour address
 * @param[in] tree_level neighbour tree level
 * @return sync_neighbour_t* pointer to neighbour or zero
 */
sync_neighbour_t* SYNC_FindOrCreateNeighbour(dev_addr_t addr, int tree_level);

/**
 * @brief Send Poll message to start SYNC process with a given anchor
 *
 * @param[in] dst destination device address
 * @param[in] anchors ist of anchor to measure ()
 * @param[in] anc_cnt counter of anchors to measure
 * @return int
 */
int SYNC_SendPoll(dev_addr_t dst, dev_addr_t anchors[], int anc_cnt);

/**
 * @brief Send TDOA beacon message as a TAG
 *        Device go sleep after sending message from SYNC_TxCb
 *
 */
void SYNC_SendBeacon();

/**
 * @brief Process TDOA beacon message sent from TAG device
 *
 * @param[in] data pointer to FC_TDOA_BEACON_TAG_INFO_s
 * @param[in] info extra packet info
 * @return void
 */
void FC_TDOA_BEACON_TAG_INFO_cb(const void* data, const prot_packet_info_t* info);

/**
 * @brief Process TDOA beacon message sent from ANCHOR or SINK device
 *
 * @param[in] data pointer to FC_TDOA_BEACON_ANCHOR_INFO_s
 * @param[in] info extra packet info
 * @return void
 */
void FC_TDOA_BEACON_ANCHOR_INFO_cb(const void* data, const prot_packet_info_t* info);

/**
 * @brief sync rx callback
 *
 * @param[in] data pointer to data to parse
 * @param[in] info extra packet info
 * @return int 1 if packed was parser 0 otherwise
 */
int SYNC_RxCb(const void* data, const prot_packet_info_t* info);

/**
 * @brief sync rx timeout routine
 *
 * @return int 1 if packed was parsed 0 otherwise
 */
int SYNC_RxToCb();

/**
 * @brief sync tx callback
 *
 * @param[in] TsDwTx transmit timestamp in local dtu
 * @return int 1 if packed was parsed 0 otherwise
 */
int SYNC_TxCb(int64_t TsDwTx);
#endif
