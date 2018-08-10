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
#include "nrf_drv_timer.h"
#include "bootloader.h"

// do not touch
#define APP1_ADDR	(FLASH_BASE + BOOTLOADER_MAX_SIZE)
#define APP2_ADDR	(APP1_ADDR + APP_MAX_SIZE)
#define SYS_ADDR	(0x1FFF0000)

#define FSTATUS_NEW	0xFF
#define FSTATUS_WORK 0xFE
#define FSTATUS_BROKEN_ONCE 0x7F

typedef struct __attribute__((packed))
{
	const uint32_t boot_reserved;
	uint8_t hVersion;
	uint8_t fMajor;
	uint16_t fMinor;
	uint64_t hash;
} settings_edit_t;

typedef struct
{
	settings_edit_t saved_version;
	const settings_edit_t * firmware_version;
	int8_t pass_cnt, fail_cnt;
} app_set_t;

typedef struct
{
	app_set_t stat[2];
	uint8_t app_to_choose_when_equal;
} bootloader_set_t;

const bootloader_set_t  _flash_settings __attribute__((section (".settings")));
bootloader_set_t settings;
uint8_t Bootloader_PageBuffer[FLASH_PAGE_SIZE];

// array with application addresses
const void* bootloader_apps[] = {
		(void*)APP1_ADDR,
		(void*)APP2_ADDR
	};

void Bootloader_JumpApp(int index);

// implementation
void MainInit(void) {
	nrf_gpio_cfg_output(LED_B1);
	nrf_gpio_cfg_output(LED_G1);
	nrf_gpio_cfg_output(LED_R1);
	nrf_gpio_pin_set(LED_B1);
	nrf_gpio_pin_set(LED_G1);
	nrf_gpio_pin_set(LED_R1);
}

void MainDeinit(void) {
	nrf_gpio_cfg_default(LED_B1);
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

static void Bootloader_SaveSettings()
{
	bootloader_set_t new_set = settings;
	uint32_t len = sizeof(bootloader_set_t);
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

const nrf_drv_timer_t BL_TIMER = NRF_DRV_TIMER_INSTANCE(2);
static void bl_timer_event_handler(nrf_timer_event_t event_type, void* p_context) {}

void Bootloader_StartIWDG()
{
    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    nrf_drv_timer_init(&BL_TIMER, &timer_cfg, bl_timer_event_handler);
    nrf_drv_timer_extended_compare(
         &BL_TIMER, NRF_TIMER_CC_CHANNEL0, nrf_drv_timer_ms_to_ticks(&BL_TIMER, 8000), NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
    nrf_drv_timer_enable(&BL_TIMER);
}

void Bootloader_MarkFirmwareAsOld(int app)
{
	const uint32_t addr_to_write = (uint32_t)bootloader_apps[app]+APP_TO_SETTINGS_OFFSET;
	const uint32_t addr_to_read = (uint32_t)Bootloader_PageBuffer;
	const int size = FLASH_PAGE_SIZE;
	const uint32_t magic = BOOTLOADER_MAGIC_NUMBER;

	memcpy((uint8_t*)addr_to_read, (uint8_t*)addr_to_write, size); // copy flash content
	memcpy((uint8_t*)(addr_to_read), &magic, sizeof(magic)); // and set magic number at the beginning

	nrf_delay_ms(100); // to wait for eventually watchdogs before

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

int Bootloader_CheckNewFirmware(int app)
{
	app_set_t* pset = &settings.stat[app];

	bool new_ver = 0;
	// check magic number at the end of a first page
	uint32_t* ptr = (uint32_t*)(APP1_ADDR+APP_MAX_SIZE*app+APP_TO_SETTINGS_OFFSET);
	if(*ptr != BOOTLOADER_MAGIC_NUMBER) {
		Bootloader_MarkFirmwareAsOld(app);
		new_ver = 1;
	}
	// if firmware changed
	// ommit 1 byte - hadrware version
	if(memcmp((uint8_t*)&pset->firmware_version->fMajor, (uint8_t*)&pset->saved_version.fMajor, sizeof(*pset->firmware_version)-5) != 0) {
		new_ver = 1;
	}

	if(new_ver){
		// coppy version
		memcpy(&pset->saved_version, pset->firmware_version, sizeof(pset->saved_version));
		// set each counters to 0
		for(int i = 0; i < 2; ++i) {
			settings.stat[i].fail_cnt = 0;
			settings.stat[i].pass_cnt = 0;
		}
		// prepare to run app next time
		Bootloader_WriteSpecialReg(0);			// only when new app appears
		Bootloader_BkpSave(0);
		// start IWDG
		Bootloader_SaveSettings();
		Bootloader_StartIWDG();
		Bootloader_JumpApp(app);
	}
	return new_ver;
}

int Bootloader_CheckHardwareVersions()
{
	uint8_t h_ver;
	const bootloader_set_t* oryg_set = (bootloader_set_t*)&_flash_settings;
	uint8_t ver[] = {oryg_set->stat[0].saved_version.hVersion,
							oryg_set->stat[1].saved_version.hVersion
					};
	int cnt[] = {1, 1};

	// check first run before copy new api data
	int hVersionSaved = (oryg_set->stat[0].saved_version.hVersion != 0) || (oryg_set->stat[1].saved_version.hVersion != 0);

	if(hVersionSaved) {
		// counter number of each version
		for (int i = 0; i < 2; ++i) {
			if((settings.stat[i].firmware_version->hVersion&H_VER_MASK) == (ver[0]&H_VER_MASK)) { ++cnt[0]; }
			if((settings.stat[i].firmware_version->hVersion&H_VER_MASK) == (ver[1]&H_VER_MASK)) { ++cnt[1]; }
		}

		// choose right hardware version
		if(cnt[0] > cnt[1]){
			h_ver = ver[0];
		} else {
			h_ver = ver[1];
		}
	} else {
		// after first run ver[0] should be correct
		if(settings.stat[0].firmware_version->hVersion != 0xFF) {
			h_ver = settings.stat[0].firmware_version->hVersion & H_VER_MASK;
		} else if (settings.stat[1].firmware_version->hVersion != 0xFF){
			h_ver = settings.stat[1].firmware_version->hVersion & H_VER_MASK;
		} else {
			h_ver = 0; // to keep itIsFirstRun state
		}
	}

	// set it as correct h_version for each firmware
	if((settings.stat[0].saved_version.hVersion&H_VER_MASK) != (h_ver&H_VER_MASK) ||
		(settings.stat[1].saved_version.hVersion&H_VER_MASK) != (h_ver&H_VER_MASK)) {
		settings.stat[0].saved_version.hVersion = h_ver;
		settings.stat[1].saved_version.hVersion = h_ver;
		return 1;
	}
	return 0;
}

int Bootloader_CheckSettingsPointer(app_set_t* pset, const char* app_start_addr)
{
	const uint8_t* FLASH_END = (uint8_t *)FLASH_BASE + FLASH_SIZE;

	// check pointer to firmware_version
	if(!((char*)FLASH_BASE < (char*)pset->firmware_version && (char*)pset->firmware_version < (char*)FLASH_END))
	{
		pset->firmware_version = (const settings_edit_t*)(app_start_addr+APP_TO_SETTINGS_OFFSET);
		return 1;
	}
	return 0;
}

int Bootloader_UpdateAppPassFailCounter(int previous_app)
{
	app_set_t* pset = &settings.stat[0];

	//read magic number
	if(Bootloader_ReadSpecialReg() == BOOTLOADER_MAGIC_NUMBER) {
	  // correct firmware version
	  if(pset[previous_app].pass_cnt < MAX_PASS_FAIL_COUNTER) {
		  ++pset[previous_app].pass_cnt;
	  }
	  if(pset[previous_app].fail_cnt > 0) {
		  --pset[previous_app].fail_cnt;
	  }
	  return 1;
	} else {
	  // bad firmware
	  if(pset[previous_app].pass_cnt > 0) {
		  --pset[previous_app].pass_cnt;
	  }
	  if(pset[previous_app].fail_cnt < MAX_PASS_FAIL_COUNTER) {
		  ++pset[previous_app].fail_cnt;
	  }
	  return 1;
	}
	return 0;
}

static inline bool Bootloader_IfNewFirmware(uint32_t reset_reason) {	// all pass/fail counters are set to 0 whenever new application appears in flash
	return !settings.stat[0].pass_cnt && !settings.stat[1].pass_cnt && !settings.stat[0].fail_cnt && !settings.stat[1].fail_cnt; // (reset_reason & WDG_RST) &&
}

void Bootloader_Init(uint32_t reset_reason)
{
    int previous_app = *BOOTLOADER_BKP_REG - 1;
	app_set_t* pset = &settings.stat[0];
	uint8_t change_cnt = 0;

	// assert linker variable
	extern uint32_t PROG_FLASH_START, PROG_PAGE_SIZE;
	while((uint32_t)(&PROG_FLASH_START) != FLASH_BASE) {};
	while((uint32_t)(&PROG_PAGE_SIZE) != FLASH_PAGE_SIZE) {};

	memcpy(&settings, &_flash_settings, sizeof(settings));

	// check reset source
	if(Bootloader_IfNewFirmware(reset_reason)) {
		change_cnt += Bootloader_UpdateAppPassFailCounter(previous_app);
		Bootloader_BkpSave(0);
	}
	// set proper pointers in pset[i].settings
	// check if is new firmware
	change_cnt += Bootloader_CheckHardwareVersions();
	for(int i = 0; i < 2; ++i) {
		change_cnt += Bootloader_CheckSettingsPointer(&pset[i], bootloader_apps[i]);
		change_cnt += Bootloader_CheckNewFirmware(i);
	}

	if(change_cnt > 0) {
		Bootloader_SaveSettings();
	}

	// software reset - run last App
	if(*BOOTLOADER_BKP_REG != 0 && (reset_reason & SOFT_RST)) {
		Bootloader_JumpApp(previous_app);
	 }
}


static void Bootloader_JumpToAddr(long addr)
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


void Bootloader_JumpApp(int index)
{
    switch(index) {
		case 0:
			nrf_gpio_pin_clear(LED_G1);
			break;
		case 1:
			nrf_gpio_pin_clear(LED_R1);
			break;
    }
    nrf_delay_ms(50);

	Bootloader_BkpSave(index+1); // add 1 to ommit zero index
	if(index < sizeof(bootloader_apps)/sizeof(*bootloader_apps)) {
		Bootloader_JumpToAddr((long)bootloader_apps[index]);
	}
}


static int Bootloader_CheckForApplication(long addr)
{
    uint32_t ResetHandlerAddr = *(((__IO uint32_t*)addr) + 1);

    int badStackPonter = ( ((*(__IO uint32_t*)addr) & 0x2FFE0000) == 0x20000000 ) ? 0 : 1;
    int badResetHandlerAddr =  ResetHandlerAddr < addr || addr + APP_MAX_SIZE < ResetHandlerAddr;

    if(badStackPonter || badResetHandlerAddr){ return 0; }
    else return 1;
}


static int Bootloader_ScoreApplication(int i)
{
	int score = 0;
	const app_set_t* pset = &settings.stat[i];

	if(Bootloader_CheckForApplication((long)bootloader_apps[i]) != 0) {
		score += 2; // firmware is present

#if !SIMPLIFIED
		// check hardware version
		if((pset->firmware_version->hVersion&H_VER_MASK) == (pset->saved_version.hVersion&H_VER_MASK)) {
			//score += pset->firmware_version->fMajor*256+(pset->firmware_version->fMinor&~1); // preferujemy nowszy firmware
			score += 2*(pset->pass_cnt - pset->fail_cnt);

			if(score <= 0) {
				score = 1;
			}

			// discriminate malfunctioning firmware
			if(pset->fail_cnt == MAX_PASS_FAIL_COUNTER) {
				score = 0;
			}
		}
#else
		score += pset->pass_cnt - pset->fail_cnt;
		score *= score > 0;
		score += 1;
#endif
	}

	return score;
}


void Bootloader_JumpToSystem()
{
	Bootloader_JumpToAddr(SYS_ADDR); // gdy nic nie dziala
}


void Bootloader_JumpToApi()
{
	int score1, score2;

	score1 = Bootloader_ScoreApplication(0);
	score2 = Bootloader_ScoreApplication(1);

	if(score1 > score2) {
		Bootloader_JumpApp(0);
	} else if(score1 < score2) {
		Bootloader_JumpApp(1);
	} else if(score1 == 0 && score2 == 0) { // there is any api
		nrf_gpio_pin_clear(LED_G1);
		nrf_gpio_pin_clear(LED_R1);
		Bootloader_JumpToSystem();
	} else if(score1 == score2) {
		int app = settings.app_to_choose_when_equal;
		settings.app_to_choose_when_equal = (settings.app_to_choose_when_equal + 1) % 2;
		Bootloader_SaveSettings();
		Bootloader_StartIWDG();
		Bootloader_JumpApp(app);
	} else {
		nrf_gpio_pin_clear(LED_G1);
		Bootloader_JumpApp(0);
	}
}
