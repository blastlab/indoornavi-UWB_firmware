#ifndef _PORT_H
#define _PORT_H

#include "port_config.h"
#include "iassert.h"
#include "nrf_error.h"
#include "nrf52.h"

#define PORT_ASSERT(expr) IASSERT(expr)
#define UNUSED(x) ((void)(x))


//char PROG_DESTINATION1; // TODO should be defined in linker script
//char PROG_DESTINATION2;

#define FLASH_PAGE_SIZE 	NRF_FICR->CODEPAGESIZE
#define FLASH_BASE 			((uint32_t)0x0U)
#define FLASH_BANK_SIZE  	((uint32_t)0x80000U)

#define PORT_Success NRF_SUCCESS

// debug configuration
#define DBG 0

// BLE beacon - this method must be called before LFCLK initialization due to softdevice's init method
void PORT_BleBeaconStart(void);
void PORT_BleSetAdvData(uint16_t maj_val, uint16_t min_val);
void PORT_BleAdvStart(void);

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

// wake up DW1000 device by setting CS low at least 500 us
void PORT_WakeupTransceiver(void);

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

// USB data transmit
uint8_t PORT_UsbUartTransmit(uint8_t *buf, uint16_t len);

// TIME

// run timers when device is fully initialised
void PORT_TimeStartTimers();

// nop
void PORT_SleepMs(unsigned int time_ms);

// get clock
unsigned int PORT_TickMs();

// get high resolution clock - CPU tick counter
unsigned int PORT_TickHr();

// convert high resolution clock time units to us
unsigned int PORT_TickHrToUs(unsigned int delta);

// set slot timer period
void PORT_SetSlotTimerPeriodUs(uint32 us);

// get current slot timer count
uint32_t PORT_SlotTimerTick();

// extend slot timer period for one iteration by delta_us
void PORT_SlotTimerSetUsOffset(int32 delta_us);

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
void PORT_BkpRegisterWrite(uint32_t *reg, uint32_t value);

// read value from reset-safe backup register
uint32_t PORT_BkpRegisterRead(uint32_t *reg);

// clear flash pages before fill with new data
int PORT_FlashErase(void *flash_addr, uint32_t length);

// write new data to previously erased flash memory
int PORT_FlashSave(void *destination, const void *p_source, uint32_t length);

#endif