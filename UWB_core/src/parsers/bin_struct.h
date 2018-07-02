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
  uint64_t serial;  ///< device serial number from settings.version.serial
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

#endif
