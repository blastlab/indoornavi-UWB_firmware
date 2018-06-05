#ifndef _BIN_STRUCT_H
#define _BIN_STRUCT_H

#include "../mac/mac_const.h"
#include "../mac/mac_settings.h"  // rtls_role
#include "bin_const.h"
#include <stdint.h>


typedef struct __packed {
  uint8_t FC, len;
} FC_TURN_ON_s;

typedef struct __packed {
  uint8_t FC, len;
  uint8_t reason;
} FC_TURN_OFF_s;

typedef struct __packed {
  uint8_t FC, len;
  uint64_t serial;
} FC_BEACON_s;

typedef struct __packed {
  uint8_t FC, len;
  uint16_t battery_mV;
  uint16_t err_cnt, to_cnt, rx_cnt, tx_cnt;
} FC_STAT_s;

typedef struct __packed {
  uint8_t FC, len;
  uint8_t hMajor;
  uint8_t hMinor;
  uint64_t serial;
  uint8_t fMajor;
  uint16_t fMinor;
  uint64_t hash;
  rtls_role role;
} FC_VERSION_s;

#endif
