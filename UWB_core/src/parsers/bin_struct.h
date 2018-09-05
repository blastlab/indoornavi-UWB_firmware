/**
 * @brief binary data structures definition
 * 
 * Each data frame need to start with Function Code field (FC 1 byte) 
 * followed by frame len (len 1 byte).
 * 
 * @file bin_struct.h
 * @author Karol Trzcinski
 * @date 2018-06-28
 */
#ifndef _BIN_STRUCT_H
#define _BIN_STRUCT_H

#include "../mac/mac_const.h"
#include "../mac/mac_settings.h"  // rtls_role
#include "bin_const.h"
#include <stdint.h>

/**
 * @brief see #FC_t description
 * 
 */
typedef struct __packed {
  uint8_t FC, len;
} FC_TURN_ON_s;

/**
 * @brief see #FC_t description
 * 
 */
typedef struct __packed {
  uint8_t FC, len;
  uint8_t reason;  ///< reason for turn OFF
} FC_TURN_OFF_s;

/**
 * @brief see #FC_t description
 * 
 */
typedef struct __packed {
  uint8_t FC, len;
  uint8_t hop_cnt;
  uint8_t padding;
  uint64_t serial;  ///< device serial number from settings.version.serial
  dev_addr_t src_did;
  dev_addr_t hops[0];
} FC_BEACON_s;

/**
 * @brief see #FC_t description
 * 
 */
typedef struct __packed {
  uint8_t FC, len;
  uint16_t battery_mV;  ///< last measured battery voltage in millivolt
  uint16_t err_cnt;  ///< radio transmission and reception error counter
  uint16_t to_cnt;  ///< radio transmission timeout counter
  uint16_t rx_cnt;  ///< radio reception counter
  uint16_t tx_cnt;  ///< radio successful transmission counter 
} FC_STAT_s;

/**
 * @brief see #FC_t description
 * 
 */
typedef struct __packed {
  uint8_t FC, len;
  uint8_t hMajor;  ///< hardware major version
  uint8_t hMinor;  ///< hardware minor version 
  uint8_t role;  ///< device role, #rtls_role
  uint8_t fMajor;  ///< firmware major version
  uint16_t fMinor;  ///< firmware minor version
  uint64_t serial;  ///< device serial number
  uint64_t hash;  ///< firmware git hash commit
} FC_VERSION_s;

/**
 * @brief see #FC_t description
 * 
 */
typedef struct __packed {
  uint8_t FC, len;
  dev_addr_t newParent;
} FC_DEV_ACCEPTED_s;

/**
 * @brief see #FC_t description
 *
 */
typedef struct __packed {
  uint8_t FC, len;
  uint8_t result;  ///< 0 success, 1 erasing error, 2 writing error
  uint8_t padding;
} FC_SETTINGS_SAVE_RESULT_s;

/**
 * @brief see #FC_t description
 *
 */
typedef struct __packed {
  uint8_t FC, len;
  uint8_t chan;    ///< rf channel number {1,2,3,4,5,7}, change frequency and bw
  uint8_t pac;     ///< * Acquisition Chunk Size (Relates to RX preamble length)
  uint8_t plen;    ///< DWT_PLEN_64..DWT_PLEN_4096
  uint8_t br;      ///< Baund Rate {DWT_BR_110K, DWT_BR_850K or DWT_BR_6M8}
  uint8_t code;    ///< TX and RX preamble code 1..24
  uint8_t ns_sfd;  ///< * Boolean should we use non-standard SFD for better performance
  uint32_t power;  ///< *31:24     BOOST_0.125ms_PWR
                   ///< 23:16     BOOST_0.25ms_PWR-TX_SHR_PWR
                   ///< 15:8      BOOST_0.5ms_PWR-TX_PHR_PWR
                   ///< 7:0       DEFAULT_PWR-TX_DATA_PWR
  uint16_t sfd_to; ///< SFD timeout value (in symbols)
  uint8_t pg_dly;  ///< rf Pulse Generator delay, adjust rf bandwidth
  uint8_t prf;     ///< change rf pulse repetition frequency DWT_PRF_
} FC_RF_SET_s;

typedef struct __packed {
	uint8_t FC, len;
	int8_t tx_power;
	uint8_t is_enabled;
} FC_BLE_SET_s;
#endif
