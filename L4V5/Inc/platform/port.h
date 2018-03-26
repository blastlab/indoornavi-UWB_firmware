#ifndef _PORT_H
#define _PORT_H

#include "decadriver/deca_device_api.h" // decaIrqStatus_t
#include "stm32l4xx_hal.h"
#include "usbd_cdc_if.h"
#include <stdbool.h>
#include <stdint.h>

#define HARDWARE_MAJOR 0
#define HARDWARE_MINOR 1
#define HARDWARE_UID_64 (*(uint64_t*)(0x1FFF7590))
#define HARDWARE_OTP_ADDR 0x1FFF7000

#include "iassert.h"
#define PORT_ASSERT(expr) IASSERT(expr)

#define LOG_USB_EN 1
#define LOG_SD_EN 0
#define LOG_USB_UART 0

// define how many high resolution clock tick is in one us
#define PORT_TICKS_HR_PER_US 1000
#define DW_EXTI_IRQn EXTI0_IRQn

#define BOOTLOADER_MAGIC_NUMBER (0xBECA95)
#define BOOTLOADER_MAGIC_REG (RTC->BKP0R)
#define BOOTLOADER_MAGIC_REG_GO_SLEEP (0x12345678)

// leds
#define LED_G1 1
#define LED_R1 2
#define LED_STAT LED_G1
#define LED_ERR LED_R1

// extra initialization for port modules
void port_init();

// turn led on
void port_led_on(int LED_x);

// turrn led off
void port_led_off(int LED_x);

// reset dw 1000 device by polling RST pin down for at least 500us
void reset_DW1000();

// reset STM
void port_reboot();

// turn on low power or stop mode
void port_enter_stop_mode();

// start watchdog work
void port_watchdog_init();

// refresh watchdog timer
void port_watchdog_refresh();

// measure current battery voltage
void port_battery_measure();

// return last battery voltage in [mV]
int port_battery_voltage();

// TIME

// nop
void port_sleep_ms(unsigned int time_ms);

// get clock
unsigned int port_tick_ms();

// get high resolution clock - CPU tick counter
unsigned int port_tick_hr();

// get high resolution clock frequency
unsigned int port_freq_hr();

// CRC

// set inital value to the crc calculator
void port_crc_reset();

// feed crc calculator with new data and return result
uint16_t port_crc_feed(const void *data, int size);

// MUTEX

// get deca spi mutex
decaIrqStatus_t decamutexon(void);

// release deca spi mutex
void decamutexoff(decaIrqStatus_t s);

// SPI

// set SPI speed below 3MHz when param is true or below 20MHz when false
void spi_speed_slow(bool slow);

// returns DWT_SUCCESS(0) for success or DWT_ERROR for error
int readfromspi(uint16_t headerLength, const uint8_t *headerBuffer,
                uint32_t readlength, uint8_t *readBuffer);

// returns DWT_SUCCESS(0) for success or DWT_ERROR for error
int writetospi(uint16_t headerLength, const uint8_t *headerBuffer,
               uint32_t bodylength, const uint8_t *bodyBuffer);

// FLASH

// czysci rejon strony flasha pod nowy firmware
int port_flash_erase(void *flash_addr, uint32_t length);

// zapisuje ilosc bajtow we flashu pod wskazany adres, length % 8 musi byc rowne
// 0
int port_flash_save(void *destination, const void *p_source, uint32_t length);

#endif
