#ifndef _PORT_H
#define _PORT_H

#include "port_config.h"
#include "stm32l4xx_hal.h"
#include "usbd_cdc_if.h"


#define HARDWARE_MAJOR 0
#define HARDWARE_MINOR 1
#define HARDWARE_UID_64 (*(uint64_t *)(0x1FFF7590))
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
void PORT_Init();

// turn led on
void PORT_LedOn(int LED_x);

// turrn led off
void PORT_LedOff(int LED_x);

// reset dw 1000 device by polling RST pin down for at least 500us
void PORT_ResetTransceiver();

// reset STM
void PORT_Reboot();

// turn on low power or stop mode
void PORT_EnterStopMode();

// start watchdog work
void PORT_WatchdogInit();

// refresh watchdog timer
void PORT_WatchdogRefresh();

// measure current battery voltage
void PORT_BatteryMeasure();

// return last battery voltage in [mV]
int PORT_BatteryVoltage();

// TIME

// nop
void PORT_SleepMs(unsigned int time_ms);

// get clock
unsigned int PORT_TickMs();

// get high resolution clock - CPU tick counter
unsigned int PORT_TickHr();

// get high resolution clock frequency
unsigned int PORT_FreqHr();

// CRC

// set inital value to the crc calculator
void PORT_CrcReset();

// feed crc calculator with new data and return result
uint16_t PORT_CrcFeed(const void *data, int size);

// MUTEX

// get deca spi mutex
decaIrqStatus_t decamutexon(void);

// release deca spi mutex
void decamutexoff(decaIrqStatus_t s);

// SPI

// set SPI speed below 3MHz when param is true or below 20MHz when false
void PORT_SpiSpeedSlow(bool slow);

// returns DWT_SUCCESS(0) for success or DWT_ERROR for error
int readfromspi(uint16_t headerLength, const uint8_t *headerBuffer,
                uint32_t readlength, uint8_t *readBuffer);

// returns DWT_SUCCESS(0) for success or DWT_ERROR for error
int writetospi(uint16_t headerLength, const uint8_t *headerBuffer,
               uint32_t bodylength, const uint8_t *bodyBuffer);

// FLASH

// save value in reset-safe backup register
void PORT_BkpRegisterWrite(uint32_t reg, uint32_t value);

// read value from reset-safe backup register
uint32_t PORT_BkpRegisterRead(uint32_t reg);

// clear flash pages before fill with new data
int PORT_FlashErase(void *flash_addr, uint32_t length);

// write new data to previously erased flash memory
int PORT_FlashSave(void *destination, const void *p_source, uint32_t length);

// IMU

void PORT_ImuWriteRegister(uint8_t addr, uint8_t val);

void PORT_ImuReadRegister(uint8_t addr, uint8_t *val, uint16_t count);

void PORT_ImuReset(void);

#endif
