/*
 * bootloader.h
 *
 *  Created on: 31.10.2017
 *      Author: Karol Trzcinski
 */
#include <stdbool.h>

#include "bootloader_conf.h"

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#define MAX_PASS_FAIL_COUNTER	4

typedef struct {
	const uint16_t hType;
	const uint8_t hMajor, hMinor;
	const uint64_t serial;
}__packed settings_otp_t;

typedef struct {
	const uint32_t boot_reserved;
	uint16_t hType;
	uint8_t hMajor;
	uint8_t hMinor;
	uint8_t hVersion;
	uint8_t fMajor;
	uint16_t fMinor;
	uint64_t hash;
}__packed settings_edit_t;

typedef struct {
	const settings_edit_t * firmware_version;
	int8_t pass_cnt, fail_cnt;
} app_set_t;

typedef struct {
	app_set_t stat[2];
	uint8_t app_to_choose_when_equal;
	int my_hType;
} bl_set_t;

extern const void* bl_apps_addr[];

void Start(uint32_t RCC_CSR);

///
void Bootloader_Led(LED_e led, LED_s stat);
void Bootloader_JumpToAddr(long addr);
void Bootloader_JumpToSystem();
int Bootloader_ReadSpecialReg();
void Bootloader_WriteSpecialReg(int val);
void Bootloader_BkpSave(int val);
void Bootloader_ProtectFlash(bool enable);
void Bootloader_SaveSettings();
void Bootloader_StartIWDG();
void Bootloader_MarkFirmwareAsOld(int app);
int CheckNewFirmware(int app);
void JumpToAddr(long addr);
bool IsSoftResetFromApp(uint32_t RCC_CSR);
bool SomeAppWasRunnig();

#endif /* BOOTLOADER_H_ */
