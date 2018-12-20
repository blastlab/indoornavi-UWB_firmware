/*
 * bootloader.c
 *
 *  Created on: 02.10.2017
 *      Author: KarolTrzcinski
 */

#include <stdbool.h>
#include <string.h> // memcpy, memcmp
#include "stm32l4xx_hal.h"
#include "bootloader.h"


#define FSTATUS_NEW	0xFF
#define FSTATUS_WORK 0xFE
#define FSTATUS_BROKEN_ONCE 0x7F


uint8_t Bootloader_PageBuffer[FLASH_PAGE_SIZE];

void Bootloader_Led(LED_e led, LED_s stat) {
	static GPIO_TypeDef * LED_PORTS[2] =
	    { [LED_GREEN] = LED_G1_GPIO_Port, [LED_RED
	    ] = LED_R1_GPIO_Port };
	const static uint32_t LED_PINS[2] = { [LED_GREEN] = LED_G1_Pin, [LED_RED] = LED_R1_Pin };

	if (led > sizeof(LED_PORTS) / sizeof(*LED_PORTS)) {
		while (1) {
			//assercja
		}
	}

	if (stat == LED_ON) {
		LL_GPIO_ResetOutputPin(LED_PORTS[led], LED_PINS[led]);
	} else {
		LL_GPIO_SetOutputPin(LED_PORTS[led], LED_PINS[led]);
	}
}

// implementtion
int Bootloader_ReadSpecialReg()
{
	return RTC->BKP0R;
}

void Bootloader_WriteSpecialReg(int val)
{
	HAL_PWR_EnableBkUpAccess();
	BOOTLOADER_MAGIC_REG = val;
	HAL_PWR_DisableBkUpAccess();
}

void Bootloader_BkpSave(int val)
{
	HAL_PWR_EnableBkUpAccess();
	*BOOTLOADER_BKP_REG = val;
	HAL_PWR_DisableBkUpAccess();
}

void Bootloader_ProtectFlash(bool enable)
{
	return;
	FLASH_OBProgramInitTypeDef pOBInit;

	HAL_FLASHEx_OBGetConfig(&pOBInit);
	pOBInit.OptionType = OPTIONBYTE_WRP;
	pOBInit.WRPArea = OB_WRPAREA_BANK1_AREAA;
	pOBInit.WRPStartOffset = 0;
	pOBInit.WRPEndOffset = BOOTLOADER_MAX_SIZE/FLASH_PAGE_SIZE-1;

	pOBInit.OptionType |= OPTIONBYTE_RDP;
	pOBInit.RDPLevel = OB_RDP_LEVEL_0;

	if(enable == false) {
		if(pOBInit.WRPStartOffset > 0) {
			pOBInit.WRPEndOffset = pOBInit.WRPStartOffset-1;
		} else {
			pOBInit.WRPStartOffset = pOBInit.WRPEndOffset+1;
		}
	}

	// Clear OPTVERR bit set on virgin samples
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	HAL_FLASH_OB_Unlock();
	HAL_FLASHEx_OBProgram(&pOBInit);
	HAL_FLASH_OB_Launch();
	HAL_FLASH_OB_Lock();
}

void Bootloader_SaveSettings()
{
	extern bl_set_t settings;
	extern const bl_set_t* _flash_settings;
	bl_set_t new_set = settings;

	// clear page
	uint32_t erase_error;
	FLASH_EraseInitTypeDef feitd;
	feitd.TypeErase = FLASH_TYPEERASE_PAGES;
	feitd.Banks = FLASH_BANK_1;
	feitd.NbPages = 1;
	feitd.Page = (uint32_t)(((uint32_t)(&_flash_settings)-FLASH_BASE) / FLASH_PAGE_SIZE);
	HAL_FLASH_Unlock();
	Bootloader_ProtectFlash(false);
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
	HAL_StatusTypeDef res = HAL_FLASHEx_Erase(&feitd, &erase_error);
	if(res == HAL_OK) {
		// write settings
		uint64_t data;
		for(int i = 0; i < sizeof(new_set); i += 8) {
			data = *((uint64_t*)((uint8_t*)&new_set + i));
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)((uint8_t*)&_flash_settings+i), data);
		}
	}
	Bootloader_ProtectFlash(true);
	HAL_FLASH_Lock();

	// check result
	if(memcmp(&_flash_settings, &new_set, sizeof(new_set)) != 0) {
		Bootloader_SaveSettings();
	}
}

void Bootloader_StartIWDG()
{
	extern IWDG_HandleTypeDef hiwdg;
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_64; //IWDG_PRESCALER_256;
	hiwdg.Init.Window = 4095;
	hiwdg.Init.Reload = 4095;
	HAL_IWDG_Init(&hiwdg);
}

void Bootloader_MarkFirmwareAsOld(int app)
{
	const uint32_t addr_to_write = (uint32_t)bl_apps_addr[app] + APP_TO_SETTINGS_OFFSET;
	const uint32_t addr_to_read = (uint32_t)Bootloader_PageBuffer;
	const int size = FLASH_PAGE_SIZE;
	const uint32_t magic = BOOTLOADER_MAGIC_NUMBER;
	uint32_t erase_error;
	FLASH_EraseInitTypeDef feitd;
	feitd.TypeErase = FLASH_TYPEERASE_PAGES;
	feitd.Banks = FLASH_BANK_1;
	feitd.NbPages = 1;
	feitd.Page = (uint32_t)((addr_to_write-FLASH_BASE) / FLASH_PAGE_SIZE);
	memcpy((uint8_t*)addr_to_read, (uint8_t*)addr_to_write, size); // coppy flash content
	memcpy((uint8_t*)(addr_to_read), &magic, sizeof(magic)); // and set magic number at the beginning

	HAL_Delay(100); // to wait for eventually watchdogs before
	__disable_irq();

	do {
		HAL_FLASH_Unlock();
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
		HAL_StatusTypeDef res = HAL_FLASHEx_Erase(&feitd, &erase_error);
		if(res == HAL_OK) {
			// write settings
			uint64_t data;
			for(int i = 0; i < size; i += 8) {
				data = *((uint64_t*)(addr_to_read + i));
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)(addr_to_write+i), data);
			}
		}
		HAL_FLASH_Lock();
	// check result
	} while(memcmp((void*)addr_to_write, (void*)addr_to_read, size) != 0);

	__enable_irq();
}

void Bootloader_JumpToAddr(long addr) {
	typedef void (*pFunction)(void);
	uint32_t JumpAddress = *(__IO uint32_t*)(addr + 4);
	pFunction Jump = (pFunction)JumpAddress;

	USB->BCDR &= ~(USB_BCDR_DPPU); // turn off DP pull-up ->disconnect USB
	HAL_Delay(10);
	HAL_RCC_DeInit();
	HAL_DeInit();

	HAL_Delay(50); // swiecenie led'ami

	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;

	__set_MSP(*(__IO uint32_t*)addr);
	Jump();
}

void Bootloader_JumpToSystem() {
	Bootloader_JumpToAddr(SYS_ADDR); // gdy nic nie dziala
}

bool IsSoftResetFromApp(uint32_t RCC_CSR) {
	//const int flags = RCC_CSR_IWDGRSTF | RCC_CSR_BORRSTF | RCC_CSR_PINRSTF;
	//bool IsSoftReset = (READ_BIT(RCC_CSR, flags) == 0;
	bool IsSoftReset = READ_BIT(RCC_CSR, RCC_CSR_SFTRSTF) == RCC_CSR_SFTRSTF;
	return SomeAppWasRunnig() && IsSoftReset;
}
