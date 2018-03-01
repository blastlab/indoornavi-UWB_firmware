#ifndef _TRANSCEIVER_H
#define _TRANSCEIVER_H

#include <stdint.h>

#include "transceiver_settings.h"

#define UUS_TO_DWT_TIME 1
#define MASK_40BIT 0xffffffffff

int transceiver_init();

// immediately send data via transceiver
int transceiver_send(const void *buf, unsigned int len);

// estimate time in us for transmission packet
int transceiver_estimate_tx_time_us(unsigned int len);

// return time in transceiver time units
uint64_t transceiver_get_sync_time();

#endif // _TRANSCEIVER_H
