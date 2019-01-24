/*
 * bootloader_conf.h
 *
 *  Created on: 20.12.2018
 *      Author: KarolTrzcinski
 */

#include "stm32l4xx_hal.h"

#ifndef BOOTLOADER_CONF_H_
#define BOOTLOADER_CONF_H_

/// when new firmware know this number then it is correct firmware
#define BOOTLOADER_MAGIC_NUMBER	(0xBECA95)

/// when it is new firmware
#define BOOTLOADER_NEW_FIRMWARE_NUMBER (0xDEB12)

/// place when BOOTLOADER_MAGIC_NUMBER should be stored, used by function Bootloader_WriteSpecialReg
#define BOOTLOADER_MAGIC_REG	(RTC->BKP0R)
#define BOOTLOADER_BKP_REG		((uint32_t*)(&RTC->BKP1R))

#define BOOTLOADER_MAX_SIZE  	(32*1024)
#define APP_TO_SETTINGS_OFFSET	(FLASH_PAGE_SIZE)
#define OTP_ADDR							(0x1FFF)
#define APP_MAX_SIZE 					((256*1024 - BOOTLOADER_MAX_SIZE)/2) // 256kB flash size

// do not touch
#define APP1_ADDR	(FLASH_BASE + BOOTLOADER_MAX_SIZE)
#define APP2_ADDR	(APP1_ADDR + APP_MAX_SIZE)
#define SYS_ADDR	(0x1FFF0000)

typedef enum {
	LED_RED,
	LED_GREEN,
} LED_e;

typedef enum {
	LED_ON = 1,
	LED_OFF = 0,
} LED_s;

#endif /* BOOTLOADER_CONF_H_ */
