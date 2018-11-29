/**
 * \file
 * \brief Portable module, strictly hardware dependent.
 * \author Karol Trzcimski
 * \date 06-2018
 *
 * Each function from this module has to be implemented
 * in current hardware project workspace.
 * To fully port project you need to implement each function
 * from this header and connect interrupts routines, especially:
 *   - #dwt_isr
 *   - #MAC_YourSlotIsr
 *
 */

#ifndef _PORT_H
#define _PORT_H

#include "mac/mac_const.h"  // rtls_role
#include "platform/port_config.h"

#include "iassert.h"
#define PORT_ASSERT(expr) IASSERT(expr)

// define typedef macro before including
// deca_device api
#ifndef uint32
#define uint32 uint32_t
#endif
#ifndef uint16
#define uint16 uint16_t
#endif
#ifndef uint8
#define uint8 uint8_t
#endif

#include "decadriver/deca_device_api.h"

#ifndef STATIC
#ifdef TEST
#define STATIC
#else
#define STATIC static
#endif
#endif

/**
 * \brief This is used to set or reset debug mode
 *
 * Especially difference is during assertion.
 * In debug mode assert lead to IC hang and in release mode to reset.
 */
#define DBG 1
#define USE_SLOT_TIMER 0

/**
 * \brief This is a trace enums, useful to track application behavior
 */
typedef enum {
	TRACE_EMPTY = 0,
	TRACE_SYSTICK = 1,
	TRACE_PREPARE_SLEEP,
	TRACE_GO_SLEEP,
	TRACE_WAKEUP,
	TRACE_DW_IRQ_ENTER,
	TRACE_DW_IRQ_RX,
	TRACE_DW_IRQ_TX,
	TRACE_DW_IRQ_TO,
	TRACE_DW_IRQ_ERR,
	TRACE_DW_IRQ_EXIT,
	TRACE_SLOT_TIM_ENTER,
	TRACE_SLOT_TIM_EXIT,
	TRACE_WAKE_TIM_ENTER,
	TRACE_WAKE_TIM_EXIT,
	TRACE_IMU_IRQ_ENTER,
	TRACE_IMU_IRQ_EXIT,
	TRACE_USART_IRQ_ENTER,
	TRACE_USART_IRQ_EXIT,
} TRACE_t;

/**
 * \brief Initialization for port modules
 *
 * Especially difference is during assertion.
 * In debug mode assert lead to IC hang and in release mode to reset.
 *
 * \return void
 */
void PORT_Init();

// BLE beacon - this method must be called before LFCLK initialization due to
// softdevice's init method
void PORT_BleBeaconInit(void);
void PORT_SetUwbMeasuresAdv(uint8_t* meas_addr);
void PORT_BleSetAdvData(uint16_t maj_val, uint16_t min_val, int8_t rssi_at_m);
void PORT_BleAdvStart(void);
void PORT_BleAdvStop(void);
bool PORT_BleIsEnabled(void);
void PORT_BleSetPower(int8_t power);

/**
 * \brief assert routine function.
 *
 * It is recommended to make implementation of this function
 * depended from DBG flag. By default it should reset device
 * in release mode (to improve system reliability) and hang
 * device in debug mode (to highlight the issue).
 * Moreover in both mode it is recommended to log assert function name
 * and line.
 *
 * \param[in] msg is pointer to text message with assert function name
 * \param[in] line of assert in code
 * \return void
 */
void PORT_Iassert_fun(const char* msg, int line);

/**
 * \brief Turn led on.
 *
 * From UWB_Core there is used LED_STAT and LED_ERR macros
 * (defined in port_config.h file) to indicate current program state.
 *
 * \param[in] LED_x is hardware dependent numerical description of led.
 *
 * \return void
 */
void PORT_LedOn(int LED_x);

/**
 * \brief Turn led off.
 *
 * From UWB_Core there is used LED_STAT and LED_ERR macros
 * (defined in port_config.h file) to indicate current program state.
 *
 * \param[in] LED_x is hardware dependent numerical description of led.
 *
 * \return void
 */
void PORT_LedOff(int LED_x);

/**
 * \brief Hard reset DW1000 device by RST pin.
 *
 * \note Keep RST pin down for at leas 500 microseconds.
 *
 * \return void
 */
void PORT_ResetTransceiver();

/**
 * \brief Wake up DW1000 device.
 *
 * It can be done by setting DW1000 CS pin low for at least
 * 500 microseconds.
 *
 * \return void
 */
void PORT_WakeupTransceiver(void);

/**
 * \brief Reset host microcontroler.
 *
 * Used especially after successful firmware upgrade to change
 * working firmware. Also assert function can use it.
 *
 * \return void
 */
void PORT_Reboot();

/**
 * \brief Prepare uC to enter sleep-mode
 *
 * It turns off peripherals and minimize current consumption
 *
 */
void PORT_PrepareSleepMode();

/**
 * \brief Reinitialize uC after sleep-mode
 *
 * It turns peripherals back on and prepares uC to work
 *
 */
void PORT_ExitSleepMode();

/**
 * \brief turn on low power or stop mode.
 *
 * PORT_PrepareSleepMode should be used before
 *
 * \return return true if device should be in sleep mode and false otherwise
 */
bool PORT_EnterSleepMode();

/**
 * \brief enter low power run mode
 *
 * Used especially with tag.
 */
void PORT_LowPowerRun();

/**
 * \brief Start watchdog work
 *
 * Watchdog is refreshed in each iteration of main loop.
 * Also in PORT_Sleep and PORT_FlashErase and PORT_FlashSave this
 * function should be used.
 *
 * \return void
 */
void PORT_WatchdogInit();

/**
 * \brief Refresh watchdog timer
 *
 * Watchdog is refreshed in each iteration of main loop.
 * Also in PORT_Sleep and PORT_FlashErase and PORT_FlashSave this
 * function should be used.
 *
 * \return void
 */
void PORT_WatchdogRefresh();

/**
 * \brief Start battery measurement process
 *
 * \return void
 */
void PORT_BatteryMeasure();

/**
 * \brief Return last battery voltage in [mV]
 *
 * \return last battery voltage in [mV]
 */
int PORT_BatteryVoltage();

/**
 * \brief Return device role, base on HW select resistor or DipSwitch settings
 *
 * \note adc must be already initialized
 *
 * \return current device role
 */
rtls_role PORT_GetHwRole();

// ========  TIME  ==========

/**
 * \brief run timers when device is fully initialized.
 *
 * Especially slot and sleep timer
 *
 * \return void
 */
void PORT_TimeStartTimers();

/**
 * \brief Sleep and refresh watchdog.
 *
 * \param[in] time_ms time to sleep in milliseconds
 *
 * \return void
 */
void PORT_SleepMs(unsigned int time_ms);

/**
 * \brief Get current milliseconds timer counter value.
 *
 * \return current time in milliseconds
 */
time_ms_t PORT_TickMs();

/**
 * \brief Get high resolution clock counter value.
 *
 * In Cortex DWT_CYCCNT counter can be used.
 * This function is used only for generate user messages.
 *
 * \return high resolution clock counter value
 */
unsigned int PORT_TickHr();

/**
 * \brief convert high resolution clock time units to us
 *
 * \param[in] delta time difference in high resolution clock units
 *
 * \return time difference in microseconds
 */
unsigned int PORT_TickHrToUs(unsigned int delta);

/**
 * \brief return current slot timer tick counter in milliseconds
 *
 * It is used during slot timer calibration to save timestamp.
 * In systems when it is impossible to read slot timer counter value
 * it is possible to return always zero and in function
 * #PORT_SlotTimerSetUsOffset treat input parameters as an absolute value,
 * but in this form there will be degradation of precision especially
 * during heavy load.
 *
 * \return current slot timer tick counter in milliseconds
 */
uint32_t PORT_SlotTimerTickUs();

/**
 * \brief return current slot timer tick counter in milliseconds
 *
 * It is used during slot timer calibration.
 * In systems when it is impossible to read slot timer counter value
 * it is possible to return always zero in function #PORT_SlotTimerTickUs
 * and in function and treat input parameters as an absolute value,
 * but in this form there will be degradation of precision especially
 * during heavy load.
 *
 * local slot timer value counter should be updated in form:
 * 	local_cnt += delta_us
 * In this function it should be checked if this offset is possible to realize
 * and if it doesn't impact on system reliability. Watch out about timer
 * overflow and underflow.
 *
 * \param[in] delta_us time difference between global and local slot timer value
 * in microseconds
 *
 * \return void
 */
void PORT_SlotTimerSetUsOffset(int32_t delta_us);

/**
 * \brief set new slot timer period time
 *
 * \note watch out when new period is shorten than previous one.
 *
 * \param[in] us new period duration in microseconds
 */
void PORT_SetSlotTimerPeriodUs(uint32_t us);

/**
 * \brief Set beacon timer for example as a
 * low power timer or RTC alarm with a given period
 *
 * \param time_ms interval
 */
void PORT_SetBeaconTimerPeriodMs(int time_ms);

// ========  CRC  ==========

/**
 * \brief set inital value to the crc calculator
 *
 * \note initial value for IndoorNavi is 0xFFFF
 *
 * \return void
 */
void PORT_CrcReset();

/**
 * \brief feed crc calculator with new data and return result
 *
 * \note return value should be automatically set as initial
 *    value during next iteration.
 */
uint16_t PORT_CrcFeed(const void* data, int size);

// ========  MUTEX  ==========

/**
 * \brief enable deca mutex
 *
 * Disable #dwt_isr interrupt in host processor
 *
 * \return 0 if previous status of dwt_isr was disabled, 1 otherwise
 */
decaIrqStatus_t decamutexon(void);

/**
 * \brief disable deca mutex
 *
 * Enable #dwt_isr interrupt in host processor
 *
 * \param[in] s is return value from #decamutexon() function
 *
 * \return void
 */
void decamutexoff(decaIrqStatus_t s);

/**
 * \brief disable all maskable interrupts
 *
 * \return decaIrqStatus_t previous state of interrupt mask
 */
decaIrqStatus_t PORT_EnterCritical();

/**
 * @brief enable maskbale interrupts according to previous state
 *
 * @param s previous state of interrupt mask
 */
void PORT_ExitCritical(decaIrqStatus_t s);

#define CRITICAL(_CODE_)                                 \
  {                                                      \
    decaIrqStatus_t _irq_primask = PORT_EnterCritical(); \
    _CODE_                                               \
    PORT_ExitCritical(_irq_primask);                     \
  }

// ========  SPI  ==========

/**
 * \brief change decawave SPI master clock speed
 *
 * Set SPI speed below 20MHz if \p slow is false
 * or below 3 MHz if \p slow is true
 *
 * \param[in] slow boolean value
 */
void PORT_SpiSpeedSlow(bool slow);

/**
 * \brief send header and read response from DW1000 device
 *
 * \note this function is used very frequently and should be optimized for time
 * \note SPI should work in half-duplex mode, so receiving is realized after
 * transmiting header
 *
 * \param[in] headerLength to transmit in bytes
 * \param[in] headerBuffer pointer to header data
 * \param[in] readlength length of response in bytes
 * \param[out] readBuffer pointer to response buffer
 *
 * \return 0 if success error code otherwise
 */
int readfromspi(uint16_t headerLength, const uint8_t* headerBuffer, uint32_t readlength,
                uint8_t* readBuffer);

/**
 * \brief send header and write data to DW1000 device
 *
 * \note this function is used very frequently and should be optimized for time
 *
 * \param[in] headerLength to transmit in bytes
 * \param[in] headerBuffer pointer to header data
 * \param[in] bodylength length of data in bytes
 * \param[in] bodyBuffer pointer to data buffer
 *
 * \return 0 if success error code otherwise
 */
int writetospi(uint16_t headerLength, const uint8_t* headerBuffer, uint32_t bodylength,
               const uint8_t* bodyBuffer);

void PORT_SpiTx(uint32_t length, const uint8_t* buf);

void PORT_SpiRx(uint32_t length, uint8_t* buf);

// ========  FLASH  ==========

/**
 * \brief Save value in reset-safe backup register
 *
 * \note it doesn't need to save after power down
 *
 * \param[in] reg pointer to memory address
 * \param[in] value to write into register
 *
 * \return void
 */
void PORT_BkpRegisterWrite(uint32_t* reg, uint32_t value);

/**
 * \brief Read value in reset-safe backup register
 *
 * \note it doesn't need to save after power down
 *
 * \param[in] reg pointer to memory address
 *
 * \return current value
 */
uint32_t PORT_BkpRegisterRead(uint32_t* reg);

/**
 * \brief Erase memory in flash
 *
 * It is used before save new settings and during firmware upgrade
 *
 * \note watch about watchdog timeout
 *
 * \param[in] flash_addr start address of region to erase
 * \param[in] length minimal length of memory region to erase
 *
 * \return 0 if success, error code otherwise
 */
int PORT_FlashErase(void* flash_addr, uint32_t length);

/**
 * \brief Save data in flash
 *
 * It is used to save new settings and during firmware upgrade
 *
 * \note watch about watchdog timeout
 *
 * \param[in] destination start address of region to save in flash region
 * \param[in] p_source is pointer to input data buffer
 * \param[in] length minimal length of memory region to erase
 *
 * \return 0 if success, error code otherwise
 */
int PORT_FlashSave(void* destination, const void* p_source, uint32_t length);

// ========  IMU  ==========

/**
 * \brief Prepare uC to enter sleep-mode
 *
 * It turns off peripherals and minimize current consumption
 *
 */
void PORT_PrepareSleepMode();

/**
 * \brief Reinitialize uC after sleep-mode
 *
 * It turns peripherals back on and prepares uC to work
 *
 */
void PORT_ExitSleepMode();

/**
 * \brief Configure Wake-on-Motion feature
 *
 * It configures imu registers for low power cycled accelerometer mode
 * which compares samples to each other and throws interrupts
 *
 *  \note imu WoM feature is set only on TAG devices to save energy
 *
 */
void PORT_ImuInit(bool imu_available);

/**
 * \brief Check if the device should go to sleep
 *
 * It checks if the given time with no motion detected is up,
 * if so, it sets the device to sleep
 *
 */
void PORT_ImuMotionControl(bool sleep_enabled);

/**
 * \brief IMU IRQ handler
 *
 * process interrupts from IMU
 *
 */
void PORT_ImuIrqHandler();

/**
 * \brief disable IMU
 *
 */
void PORT_ImuSleep();

#endif
