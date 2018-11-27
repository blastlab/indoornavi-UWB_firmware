/**
 * @brief transceiver
 *
 * @file transceiver.h
 * @author Karol Trzcinski
 * @date 2018-06-28
 */
#ifndef _TRANSCEIVER_H
#define _TRANSCEIVER_H

#include <math.h>
#include <stdint.h>

#include "platform/port.h"
#include "settings.h"

// UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) factor.
// 1 uus = 512 / 499.2 us and 1 us = 499.2 * 128 dtu.
#define UUS_TO_DWT_TIME (65536ul)

#define MASK_40BIT (0xffffffffffL)

/**
 * @brief Initialize transceiver based on current settings
 *
 * @return device part id
 */
uint32_t TRANSCEIVER_Init();

/**
 * @brief set device address
 *
 * @param[in] pan_addr personal access network identifier
 * @param[in] dev_addr device address
 */
void TRANSCEIVER_SetAddr(pan_dev_addr_t pan_addr, dev_addr_t dev_addr);

/**
 * @brief connect event callbacks
 *
 * @param[in] tx_cb transmitted message callback
 * @param[in] rx_cb received message callback
 * @param[in] rxto_cb timeout during receiving package callback
 * @param[in] rxerr_cb error during receiving package callback
 */
void TRANSCEIVER_SetCb(dwt_cb_t tx_cb, dwt_cb_t rx_cb, dwt_cb_t rxto_cb, dwt_cb_t rxerr_cb);

/**
 * @brief move transmitter to low power mode
 *
 */
void TRANSCEIVER_EnterDeepSleep();

// turn off transmitter, wake up on chip select
void TRANSCEIVER_EnterSleep();

// waking up the device
void TRANSCEIVER_WakeUp(uint8_t *buf, int len);

// immediately send data via transceiver

/**
 * @brief move transmitter back to normal mode
 *
 */
void TRANSCEIVER_WakeUp(uint8_t *buf, int len);
/**
 * @brief immediately send data via transceiver
 *
 * @param[in] buf pointer to data buffer
 * @param[in] len data length in bytes
 * @return int 0 if success, error code otherwise
 */
int TRANSCEIVER_Send(const void *buf, unsigned int len);

/**
 * @brief send ranging data via transceiver
 *
 * @param[in] buf pointer to data buffer
 * @param[in] len data length in bytes
 * @param[in] flags flags for transceiver:
 *   - DWT_START_TX_IMMEDIATE
 *   - DWT_START_TX_DELAYED
 *   - DWT_RESPONSE_EXPECTED
 * @return int 0 if success, error code otherwise
 */
int TRANSCEIVER_SendRanging(const void *buf, unsigned int len, uint8_t flags);

/**
 * @brief read data from device
 *
 * @param[out] buf pointer to data buffer
 * @param[in] len data length in bytes
 */
void TRANSCEIVER_Read(void *buf, unsigned int len);

/**
 * @brief turn on receiver
 *
 */
void TRANSCEIVER_DefaultRx();

/**
 * @brief return time in transceiver
 *
 * @return int64_t time in transceiver time units
 */
int64_t TRANSCEIVER_GetTime();

/**
 * @brief return last packed receive timestamp
 *
 * timestamp is saved at the begin of SFD detection
 *
 * @return int64_t event timestamp in dw time units
 */
int64_t TRANSCEIVER_GetRxTimestamp(void);

/**
 * @brief return last packed transmit timestamp
 *
 * timestamp is saved at the begin of SFD transmission
 *
 * @return int64_t event timestamp in dw time units
 */
int64_t TRANSCEIVER_GetTxTimestamp(void);

/**
 * @brief read values from last rx from transceiver
 *
 * @param[out] cRSSI Received Signal Strength Indicator in centy dBm
 * @param[out] cFPP First Power Path in centy dBm
 * @param[out] cSNR Signal to Noise Ratio in centy dB
 */
void TRANSCEIVER_ReadDiagnostic(int16_t *cRSSI, int16_t *cFPP, int16_t *cSNR);

// estimate time in us for transmission packet
/**
 * @brief estimate packet transmission time
 *
 * @param[in] len packed length (without preamble, SFD and CRC len)
 * @return int estimated time in micro seconds
 */
int TRANSCEIVER_EstimateTxTimeUs(unsigned int len);

#endif // _TRANSCEIVER_H
