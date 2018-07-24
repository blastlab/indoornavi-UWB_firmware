/*
 * bootloader.h
 *
 *  Created on: 09.07.2018
 *      Author: DawidPeplinski
 */

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include <stdint.h>

#define BOOTLOADER_MAGIC_NUMBER			(0xBC)
#define BOOTLOADER_MAGIC_REG			NRF_POWER->GPREGRET
#define BOOTLOADER_BKP_REG				NRF_POWER->GPREGRET2
#define STATUS_MAGIC_NUMBER_GO_SLEEP 	(0xFC)

#define FLASH_PAGE_SIZE 			((uint32_t)0x1000U)
#define FLASH_BASE 					((uint32_t)0x23000U)
#define FLASH_BANK_SIZE  			((uint32_t)0x80000U)
#define FLASH_SIZE					((uint32_t)0x5d000)

#define BOOTLOADER_MAX_SIZE  		(32*1024)
#define APP_TO_SETTINGS_OFFSET		(FLASH_PAGE_SIZE)
#define OTP_ADDR					(0x1FFF)
#define APP_MAX_SIZE 				((256*1024 - BOOTLOADER_MAX_SIZE)/2) // TODO change to actual value // 256kB flash size

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
