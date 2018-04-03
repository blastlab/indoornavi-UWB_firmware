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

// setup
int TRANSCEIVER_Init(pan_dev_addr_t pan_addr, dev_addr_t dev_addr);

// connect event callbacks
void TRANSCEIVER_SetCb(dwt_cb_t tx_cb, dwt_cb_t rx_cb, dwt_cb_t rxto_cb,
                        dwt_cb_t rxerr_cb);

// turn off transmitter, receiver and other power consumptioning devices
void TRANSCEIVER_EnterDeepSleep();

// immediately send data via transceiver
int TRANSCEIVER_Send(const void *buf, unsigned int len);

// send ranging data via transceiver
int TRANSCEIVER_SendRanging(const void *buf, unsigned int len, uint8_t flags);

// read data from device
void TRANSCEIVER_Read(void *buf, unsigned int len);

// turn on receiver
void TRANSCEIVER_DefaultRx();

// return time in transceiver time units
int64_t TRANSCEIVER_GetTime();

// return saved event timestamp in dw time units
int64_t TRANSCEIVER_GetRxTimestamp(void);

// return saved event timestamp in dw time units
int64_t TRANSCEIVER_GetTxTimestamp(void);

// read values from last rx from transceiver and calculate values in [centy dB]
// ReceivedSignalStrengthIndicator, FirstPathPower, SignalToNoiseRatio
void TRANSCEIVER_ReadDiagnostic(int16_t *cRSSI, int16_t *cFPP, int16_t *cSNR);

// estimate time in us for transmission packet
int TRANSCEIVER_EstimateTxTimeUs(unsigned int len);

#endif // _TRANSCEIVER_H
