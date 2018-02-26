#ifndef _TRANSCEIVER_H
#define _TRANSCEIVER_H

int transceiver_init();

// immediately send data via transceiver
int transcceiver_send(const void *buf, unsigned int len);

// estimate time in us for transmission packet
int transceiver_estimate_tx_time_us(unsigned int len);

#endif // _TRANSCEIVER_H
