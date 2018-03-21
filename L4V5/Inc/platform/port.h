#ifndef _PORT_H
#define _PORT_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32l4xx_hal.h"
#include "usbd_cdc_if.h"
#include "decadriver/deca_device_api.h" // decaIrqStatus_t

#include "iassert.h"
#define PORT_ASSERT(expr) IASSERT(expr)

#define LOG_USB_EN 1
#define LOG_SD_EN 0
#define LOG_USB_UART 0

// define how many high resolution clock tick is in one us
#define PORT_TICKS_HR_PER_US 1000
#define DW_EXTI_IRQn 		EXTI0_IRQn

// refresh watchdog timer
void port_watchdog_refresh();

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
int readfromspi(uint16_t headerLength, const uint8_t *headerBuffer, uint32_t readlength, uint8_t *readBuffer);

// returns DWT_SUCCESS(0) for success or DWT_ERROR for error
int writetospi(uint16_t headerLength, const uint8_t *headerBuffer, uint32_t bodylength, const uint8_t *bodyBuffer);

// czysci rejon strony flasha pod nowy firmware
HAL_StatusTypeDef port_flash_erase(void *flash_addr, uint32_t length);

// zapisuje ilosc bajtow we flashu pod wskazany adres, length % 8 musi byc rowne 0
HAL_StatusTypeDef port_flash_save(void* destination, const void *p_source, uint32_t length);

#endif
