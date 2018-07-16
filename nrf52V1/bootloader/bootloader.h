/*
 * bootloader.h
 *
 *  Created on: 09.07.2018
 *      Author: DawidPeplinski
 */

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#define BOOTLOADER_MAGIC_NUMBER	(0xBECA95)
#define BOOTLOADER_MAGIC_REG	(RTC->BKP0R)
#define BOOTLOADER_BKP_REG		(RTC->BKP1R)

#define BOOTLOADER_MAX_SIZE  	(32*1024)
#define APP_TO_SETTINGS_OFFSET	(FLASH_PAGE_SIZE)
#define OTP_ADDR				(0x1FFF)
#define APP_MAX_SIZE 			((256*1024 - BOOTLOADER_MAX_SIZE)/2) // 256kB flash size

// mask most important hardware version bits
#define H_VER_MASK				(0xF8)

#define MAX_PASS_FAIL_COUNTER	4

void Bootloader_Init(uint32_t RCC_CSR);
void Bootloader_JumpToApi();

#define SIMPLIFIED 1

#endif /* BOOTLOADER_H_ */
