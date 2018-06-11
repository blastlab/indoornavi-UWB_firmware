#ifndef _PORT_CONST_H
#define _PORT_CONST_H

#include "decadriver/deca_device_api.h" // decaIrqStatus_t
#include <stdbool.h>
#include <stdint.h>

#define __H_MAJOR__ 0
#define __H_MINOR__ 1
#define HARDWARE_UID_64 (*(uint64_t *)(0x1FFF7590))
#define HARDWARE_OTP_ADDR 0x1FFF7000					// TODO: set OTP address

#include "iassert.h"
#define PORT_ASSERT(expr) IASSERT(expr)


#define LOG_USB_EN 1
#define LOG_SD_EN 0
#define LOG_USB_UART 0

// define how many high resolution clock tick is in one us
#define PORT_TICKS_HR_PER_US 1000

#define DW_EXTI_IRQn EXTI0_IRQn

#define BOOTLOADER_MAGIC_NUMBER (0xBECA95)
#define BOOTLOADER_MAGIC_REG 0 // ((uint32_t*)&RTC->BKP0R)
#define STATUS_MAGIC_REG 0 //((uint32_t*)&RTC->BKP1R)
#define STATUS_MAGIC_NUMBER_GO_SLEEP (0x12345678)

// leds
#define LED_G1 30
#define LED_R1 22
#define LED_B1 31
#define LED_STAT LED_G1
#define LED_ERR LED_R1
#define LED_BLE LED_B1

#endif
