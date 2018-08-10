/*
 * bootloader.h
 *
 *  Created on: 09.07.2018
 *      Author: DawidPeplinski
 */

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include <stdint.h>

#define FLASH_PAGE_SIZE 			((uint32_t)0x1000)
#define FLASH_BASE 					((uint32_t)0x23000U)
#define FLASH_BANK_SIZE  			((uint32_t)0x5C000)
#define FLASH_SIZE					FLASH_BANK_SIZE

#define BOOTLOADER_MAX_SIZE  		(32*1024)
#define APP_TO_SETTINGS_OFFSET		(FLASH_PAGE_SIZE)
#define APP_MAX_SIZE 				((FLASH_SIZE - BOOTLOADER_MAX_SIZE)/2)

#define BOOTLOADER_MAGIC_NUMBER 		(0xBECA95)
#define BOOTLOADER_MAGIC_REG_ADDR		(uint32_t)(FLASH_BASE + BOOTLOADER_MAX_SIZE - 2*FLASH_PAGE_SIZE)
#define BOOTLOADER_MAGIC_REG			(uint32_t *)BOOTLOADER_MAGIC_REG_ADDR
#define BOOTLOADER_BKP_REG_ADDR			(uint32_t)(FLASH_BASE + BOOTLOADER_MAX_SIZE - 1*FLASH_PAGE_SIZE)
#define BOOTLOADER_BKP_REG				(uint32_t *)BOOTLOADER_BKP_REG_ADDR
#define STATUS_MAGIC_NUMBER_GO_SLEEP 	(0x12345678)

// mask most important hardware version bits
#define H_VER_MASK					(0xF8)

#define MAX_PASS_FAIL_COUNTER		4

// leds
#define LED_G1 30
#define LED_R1 22
#define LED_B1 31

void MainInit(void);
void Bootloader_Init(uint32_t RCC_CSR);
void Bootloader_JumpToApi();

#define SIMPLIFIED 1

#define SHIFT_LEFT(x) 				(1 << x)
typedef enum _RESET_REASONS {
	PIN_RST		= SHIFT_LEFT(0),
	WDG_RST		= SHIFT_LEFT(1),
	SOFT_RST	= SHIFT_LEFT(2),
	LOCK_RST	= SHIFT_LEFT(3),
	OFF_RST		= SHIFT_LEFT(16),
	LPCOMP_RST	= SHIFT_LEFT(17),
	DIF_RST 	= SHIFT_LEFT(18),
	NFC_RST		= SHIFT_LEFT(19),
} RESET_REASONS;

#endif /* BOOTLOADER_H_ */
