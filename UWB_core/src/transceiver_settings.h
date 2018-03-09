#ifndef _TRANSCEIVER_SETTINGS_H
#define _TRANSCEIVER_SETTINGS_H

#include "decadriver/deca_device_api.h"

#include "iassert.h"
#define TRANSCEIVER_ASSERT(expr) IASSERT(expr)

typedef struct
{
    unsigned short ant_dly_rx, ant_dly_tx;
    char low_power_mode;
    char set_pac_from_settings;
    dwt_config_t dwt_config;
    dwt_txconfig_t dwt_txconfig;

} transceiver_settings_t;

#define TRANSCEIVER_SETTINGS_DEF                   \
    {                                              \
        .ant_dly_rx = 16436,                       \
        .ant_dly_tx = 16436,                       \
        .dwt_config.chan = 2,                      \
        .dwt_config.prf = DWT_PRF_64M,             \
        .dwt_config.txPreambLength = DWT_PLEN_128, \
        .dwt_config.rxPAC = DWT_PAC32,             \
        .dwt_config.txCode = 9,                    \
        .dwt_config.rxCode = 9,                    \
        .dwt_config.nsSFD = 1,                     \
        .dwt_config.dataRate = DWT_BR_6M8,         \
        .dwt_config.phrMode = DWT_PHRMODE_EXT,     \
        .dwt_config.sfdTO = (129 + 64 - 32),       \
        .dwt_txconfig.PGdly = 1,                   \
    \
}

#endif