#ifndef _TRANSCEIVER_H
#define _TRANSCEIVER_H

#include <stdint.h>
#include "decadriver/deca_device_api.h"

#include "settings.h"
#include "platform/port.h"

#define UUS_TO_DWT_TIME 1
#define MASK_40BIT 0xffffffffff

// setup
int transceiver_init(pan_dev_addr_t pan_addr, dev_addr_t dev_addr);

// connect event callbacks
void transceiver_set_cb(dwt_cb_t tx_cb, dwt_cb_t rx_cb, dwt_cb_t rxto_cb, dwt_cb_t rxerr_cb);

// immediately send data via transceiver
int transceiver_send(const void *buf, unsigned int len);

// turn on receiver
void transceiver_default_rx();

// return time in transceiver time units
uint64_t transceiver_get_time();

// return saved event timestamp in dw time units
uint64_t transceiver_get_rx_timestamp(void);

// return saved event timestamp in dw time units
uint64_t transceiver_get_tx_timestamp(void);

// estimate time in us for transmission packet
int transceiver_estimate_tx_time_us(unsigned int len);

#endif // _TRANSCEIVER_H
