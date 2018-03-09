#ifndef _PORT_H
#define _PORT_H

#include <stdint.h>
#include <stdbool.h>
#include "decadriver/deca_device_api.h" // decaIrqStatus_t

// define how many high resolution clock tick is in one us
#define PORT_TICKS_HR_PER_US 1000

// nop
void port_sleep_ms(unsigned int time_ms);

// get clock
unsigned int port_tick_ms();

// get high resosolution clock - CPU tick counter
unsigned int port_tick_hr();

// reset dw 1000 device by polling RST pin down for at least 500us
void reset_DW1000();

// get deca spi mutex
decaIrqStatus_t decamutexon(void);

// release deca spi mutex
void decamutexoff(decaIrqStatus_t s);

// set SPI speed below 3MHz when param is false or below 20MHz when true
void spi_speed_slow(bool fast);

// returns DWT_SUCCESS(0) for success or DWT_ERROR for error
int readfromspi(uint16 headerLength, const uint8 *headerBuffer, uint32 readlength, uint8 *readBuffer);

// returns DWT_SUCCESS(0) for success or DWT_ERROR for error
int writetospi(uint16 headerLength, const uint8 *headerBuffer, uint32 bodylength, const uint8 *bodyBuffer);

#endif