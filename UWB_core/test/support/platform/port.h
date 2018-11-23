#ifndef _PORT_H
#define _PORT_H

#include "port_config.h"

#define UNUSED(x) (void)(x)

// extra initialization for port modules
void PORT_Init();

// assert routine
void PORT_Iassert_fun(const char *msg, int line);

// turn led on
void PORT_LedOn(int LED_x);

// turrn led off
void PORT_LedOff(int LED_x);

// reset dw 1000 device by polling RST pin down for at least 500us
void PORT_ResetTransceiver();

// wakeup transceiver after sleep
void PORT_WakeupTransceiver();

// reset STM
void PORT_Reboot();

// turn on low power or stop mode
void PORT_EnterSleepMode();

// start watchdog work
void PORT_WatchdogInit();

// refresh watchdog timer
void PORT_WatchdogRefresh();

// measure current battery voltage
void PORT_BatteryMeasure();

// return last battery voltage in [mV]
int PORT_BatteryVoltage();

// TIME

// run timers when device is fully initialised
void PORT_TimeStartTimers();

// nop
void PORT_SleepMs(unsigned int time_ms);

// get clock
unsigned int PORT_TickMs();

// get high resolution clock - CPU tick counter
unsigned int PORT_TickHr();

// get high resolution clock frequency
unsigned int PORT_FreqHr();

// update slot timer for one iteration
void PORT_SlotTimerSetUsLeft(uint32 us);

// set slot timer period
void PORT_SetSlotTimerPeriodUs(uint32 us);

// CRC

// set inital value to the crc calculator
void PORT_CrcReset();

// feed crc calculator with new data and return result
uint16_t PORT_CrcFeed(const void *data, int size);

// SPI

// set SPI speed below 3MHz when param is true or below 20MHz when false
void PORT_SpiSpeedSlow(bool slow);

// FLASH

// save value in reset-safe backup register
void PORT_BkpRegisterWrite(uint32_t reg, uint32_t value);

// read value from reset-safe backup register
uint32_t PORT_BkpRegisterRead(uint32_t reg);

// clear flash pages before fill with new data
int PORT_FlashErase(void *flash_addr, uint32_t length);

// write new data to previously erased flash memory
int PORT_FlashSave(void *destination, const void *p_source, uint32_t length);

#endif
