/*
 * bootloader_conf.h
 *
 *  Created on: 20.12.2018
 *      Author: KarolTrzcinski
 */

#include "nrf52.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_soc.h"
#include "nrfx_timer.h"

#ifndef BOOTLOADER_CONF_H_
#define BOOTLOADER_CONF_H_

/*#define BOOTLOADER_MAGIC_NUMBER	(0xBECA95)

#define BOOTLOADER_MAGIC_REG	(RTC->BKP0R)
#define BOOTLOADER_BKP_REG		((uint32_t*)(&RTC->BKP1R))

#define BOOTLOADER_MAX_SIZE  	(32*1024)
#define APP_TO_SETTINGS_OFFSET	(FLASH_PAGE_SIZE)
#define OTP_ADDR							(0x1FFF)
#define APP_MAX_SIZE 					((256*1024 - BOOTLOADER_MAX_SIZE)/2) // 256kB flash size*/

// do not touch
#define APP1_ADDR	(FLASH_BASE + BOOTLOADER_MAX_SIZE)
#define APP2_ADDR	(APP1_ADDR + APP_MAX_SIZE)
#define SYS_ADDR	(0x1FFF0000)

#define FLASH_PAGE_SIZE 					((uint32_t)0x1000)
#define FLASH_BASE 								((uint32_t)0x26000U)
#define FLASH_BANK_SIZE  					((uint32_t)0x5A000)
#define FLASH_SIZE								FLASH_BANK_SIZE

#define BOOTLOADER_MAX_SIZE  			(32*1024)
#define APP_TO_SETTINGS_OFFSET		(FLASH_PAGE_SIZE)
#define APP_MAX_SIZE 							((FLASH_SIZE - BOOTLOADER_MAX_SIZE)/2)

/// when new firmware know this number then it is correct firmware
#define BOOTLOADER_MAGIC_NUMBER 		(0xBECA95)

/// place when BOOTLOADER_MAGIC_NUMBER should be stored, used by function Bootloader_WriteSpecialReg
#define BOOTLOADER_MAGIC_REG_ADDR		(uint32_t)(FLASH_BASE + BOOTLOADER_MAX_SIZE - 2*FLASH_PAGE_SIZE)
#define BOOTLOADER_MAGIC_REG			(uint32_t *)BOOTLOADER_MAGIC_REG_ADDR

/// place to store information about last launched application
#define BOOTLOADER_BKP_REG_ADDR			(uint32_t)(FLASH_BASE + BOOTLOADER_MAX_SIZE - 1*FLASH_PAGE_SIZE)
#define BOOTLOADER_BKP_REG				(uint32_t *)BOOTLOADER_BKP_REG_ADDR
#define STATUS_MAGIC_NUMBER_GO_SLEEP 	(0x12345678)

#define USE_DECA_DEVKIT 0

// leds
#if USE_DECA_DEVKIT
#define LED_G1 30
#define LED_R1 22
#else
#define LED_G1 2
#define LED_R1 15
#endif

#define SHIFT_LEFT(x) 				(1 << x)
typedef enum _RESET_REASONS {
	PIN_RST = SHIFT_LEFT(0),
	WDG_RST = SHIFT_LEFT(1),
	SOFT_RST = SHIFT_LEFT(2),
	LOCK_RST = SHIFT_LEFT(3),
	OFF_RST = SHIFT_LEFT(16),
	LPCOMP_RST = SHIFT_LEFT(17),
	DIF_RST = SHIFT_LEFT(18),
	NFC_RST = SHIFT_LEFT(19),
} RESET_REASONS;

typedef enum {
	LED_RED,
	LED_GREEN,
} LED_e;

typedef enum {
	LED_ON = 1,
	LED_OFF = 0,
} LED_s;

#endif /* BOOTLOADER_CONF_H_ */
