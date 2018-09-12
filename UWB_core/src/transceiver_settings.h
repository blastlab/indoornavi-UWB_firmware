/**
 * @brief transceiver settings struct typedef with default values
 *
 *
 *
 * @file transceiver_settings.h
 * @author Karol Trzcinski
 * @date 2018-07-02
 *
 *
 *
 * ## tutorial:
 * **chan**
 *    rf communication channel
 *    - 2 - Fc 3993.6MHz BW 499.2MHz
 *    - 5 - FC 6489.6MHz BW 499.2MHz
 *
 *  **rxPAC**
 *     Preamble Accumulation Count. This reports the number of symbols of
 *     preamble accumulated.
 *    - DWT_PAC8 for preamble len 64..128
 *    - DWT_PAC16 for preamble len 256..512
 *    - DWT_PAC32 for preamble len 1024..4096
 *
 * **nsSFD**
 *    non standard Start of Frame Delimiter
 *    - 0 for preable len < X
 *    - 1 for ptramble len >= X
 *
 * **sfdTo**
 *    Start of Frame Delimiter Timeout
 *    - 0 for automaticaly calculate
 *    - 1 + plen + sfd_len - pac
 *
 * **PGdly**
 *    Transmitter Calibration – Pulse Generator Delay.
 *    This effectively sets the width of transmitted pulses effectively
 *    setting the output bandwidth. The value used here depends on the radio TX
 *    channel selected.
 *    - 0 for autmaticaly calculate default value from datasheet
 *
 */
#ifndef _TRANSCEIVER_SETTINGS_H
#define _TRANSCEIVER_SETTINGS_H

#include "decadriver/deca_device_api.h"

#include "iassert.h"
#define TRANSCEIVER_ASSERT(expr) IASSERT(expr)

/**
 * @brief transceiver settings
 *
 */
typedef struct {
	unsigned short ant_dly_rx, ant_dly_tx;
	char low_power_mode;
	char set_pac_from_settings;
	dwt_config_t dwt_config;
	dwt_txconfig_t dwt_txconfig;

} transceiver_settings_t;

/**
 * @brief default values for transceiver settings
 *
 */
#define TRANSCEIVER_SETTINGS_DEF               \
  {                                            \
    .ant_dly_rx = 16436,                       \
    .ant_dly_tx = 16436,                       \
    .low_power_mode = 0,                       \
    .dwt_config.chan = 5,                      \
    .dwt_config.prf = DWT_PRF_64M,             \
    .dwt_config.txPreambLength = DWT_PLEN_128, \
    .dwt_config.rxPAC = DWT_PAC8,              \
    .dwt_config.txCode = 10,                    \
    .dwt_config.rxCode = 10,                    \
    .dwt_config.nsSFD = 1,                     \
    .dwt_config.dataRate = DWT_BR_6M8,         \
    .dwt_config.phrMode = DWT_PHRMODE_STD,     \
    .dwt_config.sfdTO = 0,                     \
    .dwt_txconfig.PGdly = 0,                   \
  \
}
#endif
