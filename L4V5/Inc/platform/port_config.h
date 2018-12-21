#ifndef _PORT_CONST_H
#define _PORT_CONST_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32l4xx_hal.h"
#include "usbd_cdc_if.h"

#define USE_BLE 0

#if USE_BLE
#define BLE_CODE(_CODE_) \
  { _CODE_ }
#else
#define BLE_CODE(_CODE_)
#endif

#define __H_MAJOR__ 1
#define __H_MINOR__ 5
#define __H_TYPE__  1
#define HARDWARE_UID_64 (*(uint64_t*)(UID_BASE_ADDRESS))
#define HARDWARE_OTP_ADDR 0x1FFF7000

#include "iassert.h"
#define PORT_ASSERT(expr) IASSERT(expr)
#define PORT_Success HAL_OK

#define LOG_USB_EN 1
#define LOG_SD_EN 0
#define LOG_USB_UART 0

// define how many high resolution clock tick is in one us
#define PORT_TICKS_HR_PER_US 1000

typedef unsigned int time_ms_t;

#define DW_EXTI_IRQn EXTI0_IRQn
#define DW_SLOT_TIM_IRQn TIM2_IRQn

#define BOOTLOADER_MAGIC_NUMBER (0xBECA95)
#define BOOTLOADER_MAGIC_REG ((uint32_t*)&RTC->BKP0R)
#define STATUS_MAGIC_REG ((uint32_t*)&RTC->BKP1R)
#define STATUS_MAGIC_NUMBER_GO_SLEEP (0x12345678)

// leds
#define LED_G1 1
#define LED_R1 2
#define LED_STAT LED_G1
#define LED_ERR LED_R1

#endif
