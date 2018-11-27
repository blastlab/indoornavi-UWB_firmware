#ifndef _PORT_CONST_H
#define _PORT_CONST_H

#include "decadriver/deca_device_api.h" // decaIrqStatus_t
#include <stdbool.h>
#include <stdint.h>

#define __H_MAJOR__ 0
#define __H_MINOR__ 1
#define HARDWARE_UID_64 (*(uint64_t *)(0x1FFF7590))
#define HARDWARE_OTP_ADDR 0x1FFF7000

#include "iassert.h"
#define PORT_ASSERT(expr) IASSERT(expr)

typedef unsigned int time_ms_t;

#define DW_EXTI_IRQn EXTI0_IRQn

#define BOOTLOADER_MAGIC_NUMBER (0xBECA95)
#define BOOTLOADER_MAGIC_REG (RTC->BKP0R)
#define BOOTLOADER_MAGIC_REG_GO_SLEEP (0x12345678)

// leds
#define LED_G1 1
#define LED_R1 2
#define LED_STAT LED_G1
#define LED_ERR LED_R1

#endif