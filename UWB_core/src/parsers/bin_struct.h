#ifndef _BIN_STRUCT_H
#define _BIN_STRUCT_H

#include <stdint.h>
#include "../mac/mac_const.h"
#include "bin_const.h"

typedef struct __packed {
  uint8_t FC, len;
  uint16_t err_cnt, to_cnt, rx_cnt, tx_cnt;
} FC_STAT_s;

typedef struct __packed {
  uint8_t FC, len;
  uint16_t version;
} FC_VERSION_s;

#endif
