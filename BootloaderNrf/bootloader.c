/*
 * bootloader.c
 *
 *  Created on: 09.07.2018
 *      Author: DawidPeplinski
 */

#include <stdbool.h>
#include <string.h>
#include "nrf52.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_soc.h"
#include "nrfx_timer.h"
#include "bootloader.h"

// do not touch
#define APP1_ADDR	(FLASH_BASE + BOOTLOADER_MAX_SIZE)
#define APP2_ADDR	(APP1_ADDR + APP_MAX_SIZE)
#define SYS_ADDR	(0x1FFF0000)

#define FSTATUS_NEW	0xFF
#define FSTATUS_WORK 0xFE
#define FSTATUS_BROKEN_ONCE 0x7F

extern const bl_set_t _flash_settings __attribute__((section (".settings")));
extern bl_set_t settings;
uint8_t Bootloader_PageBuffer[FLASH_PAGE_SIZE];

// array with application addresses
const void* bootloader_apps[] = {
		(void*)APP1_ADDR,
		(void*)APP2_ADDR
	};

void Bootloader_Led(LED_e led, LED_s stan) {
	if (stan == LED_ON) {
		nrf_gpio_pin_set(led);
	} else {
		nrf_gpio_pin_clear(led);
	}
}

void MainDeinit(void) {
	nrf_gpio_cfg_default(LED_G1);
	nrf_gpio_cfg_default(LED_R1);
}

int Bootloader_ReadSpecialReg()
{
	return *BOOTLOADER_MAGIC_REG;
}

void Bootloader_WriteSpecialReg(int val)
{
	if(val == *BOOTLOADER_MAGIC_REG)
		return;
	sd_flash_page_erase((uint32_t)BOOTLOADER_MAGIC_REG_ADDR/FLASH_PAGE_SIZE);
	uint32_t *dst = (uint32_t *)BOOTLOADER_MAGIC_REG_ADDR;
	uint32_t *src = (uint32_t *)&val;
	sd_flash_write(dst, src, 1);
}

void Bootloader_BkpSave(int val)
{
	if(val == *BOOTLOADER_BKP_REG)
			return;
	sd_flash_page_erase((uint32_t)BOOTLOADER_BKP_REG_ADDR/FLASH_PAGE_SIZE);
	uint32_t *dst = (uint32_t *)BOOTLOADER_BKP_REG_ADDR;
	uint32_t *src = (uint32_t *)&val;
	sd_flash_write(dst, src, 1);
}

//static void Bootloader_ProtectFlash(bool enable)
//{
//	return;
//	FLASH_OBProgramInitTypeDef pOBInit;							// TODO
//
//	HAL_FLASHEx_OBGetConfig(&pOBInit);
//	pOBInit.OptionType = OPTIONBYTE_WRP;
//	pOBInit.WRPArea = OB_WRPAREA_BANK1_AREAA;
//	pOBInit.WRPStartOffset = 0;
//	pOBInit.WRPEndOffset = BOOTLOADER_MAX_SIZE/FLASH_PAGE_SIZE-1;
//
//	pOBInit.OptionType |= OPTIONBYTE_RDP;
//	pOBInit.RDPLevel = OB_RDP_LEVEL_0;
//
//	if(enable == false) {
//		if(pOBInit.WRPStartOffset > 0) {
//			pOBInit.WRPEndOffset = pOBInit.WRPStartOffset-1;
//		} else {
//			pOBInit.WRPStartOffset = pOBInit.WRPEndOffset+1;
//		}
//	}
//
//	// Clear OPTVERR bit set on virgin samples
//	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
//	HAL_FLASH_OB_Unlock();
//	HAL_FLASHEx_OBProgram(&pOBInit);
//	HAL_FLASH_OB_Launch();
//	HAL_FLASH_OB_Lock();
//}

void Bootloader_SaveSettings()
{
	bl_set_t new_set = settings;
	uint32_t len = sizeof(bl_set_t);
	len = (len % 4) ? len + (4 - len % 4) : len;				// only full 4 bytes words can be written to flash
	// Erase settings page
	sd_flash_page_erase((uint32_t)(&_flash_settings)/FLASH_PAGE_SIZE);

	uint32_t *dst = (uint32_t *)&_flash_settings;
	uint32_t *src = (uint32_t *)&new_set;
	sd_flash_write(dst, src, len/4);

	// check result
	if(memcmp(&_flash_settings, &new_set, sizeof(new_set)) != 0) {
		Bootloader_SaveSettings();
	}
}

static const nrfx_timer_t BL_TIMER = NRFX_TIMER_INSTANCE(2);
static void bl_timer_event_handler(nrf_timer_event_t event_type, void* p_context) {}

void Bootloader_StartIWDG()
{
    nrfx_timer_config_t timer_config = NRFX_TIMER_DEFAULT_CONFIG;
    nrfx_timer_init(&BL_TIMER, &timer_config, bl_timer_event_handler);
    nrfx_timer_extended_compare(
    		&BL_TIMER, NRF_TIMER_CC_CHANNEL0, nrfx_timer_ms_to_ticks(&BL_TIMER, 8000), NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
    nrfx_timer_enable(&BL_TIMER);
}

void Bootloader_MarkFirmwareAsOld(int app)
{
	const uint32_t addr_to_write = (uint32_t)bootloader_apps[app]+APP_TO_SETTINGS_OFFSET;
	const uint32_t addr_to_read = (uint32_t)Bootloader_PageBuffer;
	const int size = FLASH_PAGE_SIZE;
	const uint32_t magic = BOOTLOADER_MAGIC_NUMBER;

	memcpy((uint8_t*)addr_to_read, (uint8_t*)addr_to_write, size); // copy flash content
	memcpy((uint8_t*)(addr_to_read), &magic, sizeof(magic)); // and set magic number at the beginning

	nrf_delay_ms(200); // to wait for eventually watchdogs before

	if(!memcmp((uint32_t*)addr_to_read, (uint32_t*)addr_to_write, size)) {
		return;
	}
	do {
		// Page erase
		sd_flash_page_erase((uint32_t)addr_to_write/FLASH_PAGE_SIZE);
		// write settings
		uint32_t *dst = (uint32_t *)addr_to_write;
		uint32_t *src = (uint32_t *)addr_to_read;
		// Enable write
	    sd_flash_write(dst, src, size/4);
	// check result
	} while(memcmp((void*)addr_to_write, (void*)addr_to_read, size) != 0);
}

bool IsSoftResetFromApp(uint32_t reset_reason) {
	bool IsSoftReset = reset_reason & SOFT_RST;
	return SomeAppWasRunnig() && IsSoftReset;
}

void Bootloader_JumpToAddr(long addr)
{
	typedef void (*pFunction)(void);
	uint32_t  JumpAddress = *(__IO uint32_t*)(addr + 4);
	pFunction Jump = (pFunction)JumpAddress;
	MainDeinit();
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL  = 0;
	__set_MSP(*(__IO uint32_t*)addr);
	Jump();
}

void Bootloader_JumpToSystem() {
	Bootloader_JumpToAddr((long)bootloader_apps[0]);
}
