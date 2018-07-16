/*
 * bootloader.c
 *
 *  Created on: 09.07.2018
 *      Author: DawidPeplinski
 */

#include <stdbool.h>
#include <string.h>
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
	const uint8_t hMajor, hMinor;
	const uint64_t serial;
} settings_otp_t;

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
bootloader_set_t settings;;

// array with application addresses
const void* bootloader_apps[] = {
		(void*)APP1_ADDR,
		(void*)APP2_ADDR
	};

int settings_offset = 9;

uint8_t Bootloader_PageBuffer[FLASH_PAGE_SIZE];


void Bootloader_JumpApp(int index);


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
	BOOTLOADER_BKP_REG = val;
	HAL_PWR_DisableBkUpAccess();
}

static void Bootloader_ProtectFlash(bool enable)
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

static void Bootloader_SaveSettings()
{
	bootloader_set_t new_set = settings;

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
	const uint32_t addr_to_write = (uint32_t)bootloader_apps[app]+APP_TO_SETTINGS_OFFSET;
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
		// start IWDG
		Bootloader_StartIWDG();
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
	const uint8_t* FLASH_END = FLASH_BASE + FLASH_SIZE;

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


void Bootloader_Init(uint32_t RCC_CSR)
{
    int previous_app = BOOTLOADER_BKP_REG-1;
	app_set_t* pset = &settings.stat[0];
	uint8_t change_cnt = 0;

	// assert linker variable
	extern uint32_t PROG_FLASH_START, PROG_PAGE_SIZE;
	while((uint32_t)(&PROG_FLASH_START) != FLASH_BASE) {};
	while((uint32_t)(&PROG_PAGE_SIZE) != FLASH_PAGE_SIZE) {};

	memcpy(&settings, &_flash_settings, sizeof(settings));
	__HAL_RCC_CLEAR_RESET_FLAGS(); // wyczysc rejestr RCC_CSR

	// software reset - run last App
	const int flags = RCC_CSR_IWDGRSTF | RCC_CSR_BORRSTF | RCC_CSR_PINRSTF;
	if(BOOTLOADER_BKP_REG != 0 && READ_BIT(RCC_CSR, flags) == 0) {
		Bootloader_JumpApp(previous_app);
	 }

	// set proper pointers in pset[i].settings
	// check if is new firmware
	for(int i = 0; i < 2; ++i) {
		change_cnt += Bootloader_CheckSettingsPointer(&pset[i], bootloader_apps[i]);
		change_cnt += Bootloader_CheckNewFirmware(i);
	}

	change_cnt += Bootloader_CheckHardwareVersions();

	// check reset source
	if(RCC_CSR & RCC_CSR_IWDGRSTF) {
		change_cnt += Bootloader_UpdateAppPassFailCounter(previous_app);
	}

	if(change_cnt > 0) {
		Bootloader_SaveSettings();
	}

	// prepare to run app next time
	Bootloader_WriteSpecialReg(0);
}


static void Bootloader_JumpToAddr(long addr)
{
	typedef void (*pFunction)(void);
    uint32_t  JumpAddress = *(__IO uint32_t*)(addr + 4);
    pFunction Jump = (pFunction)JumpAddress;

    USB->BCDR &= ~(USB_BCDR_DPPU); // turn off DP pull-up ->disconnect USB
    HAL_Delay(10);
    HAL_RCC_DeInit();
    HAL_DeInit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    __set_MSP(*(__IO uint32_t*)addr);
    Jump();
}


void Bootloader_JumpApp(int index)
{
    char* str[] = {"jump to app 1",
    				"jump to app 2"};
    UNUSED(str);

    switch(index) {
		case 0:
			LL_GPIO_SetOutputPin(LED_G1_GPIO_Port, LED_G1_Pin);
			//CDC_Transmit_FS((uint8_t*)str[0], strlen(str[0]));
			break;
		case 1:
			LL_GPIO_SetOutputPin(LED_R1_GPIO_Port, LED_R1_Pin);
			//CDC_Transmit_FS((uint8_t*)str[1], strlen(str[1]));
			break;
    }
    HAL_Delay(2);

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
		++score; // firmware is present

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
		score = pset->pass_cnt - pset->fail_cnt;
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

	if(score1 > score2){
		Bootloader_JumpApp(0);
	} else if(score1 < score2) {
		Bootloader_JumpApp(1);
	} else if(score1 == 0 && score2 == 0) { // there is any api
		LL_GPIO_SetOutputPin(LED_G1_GPIO_Port, LED_G1_Pin);
		LL_GPIO_SetOutputPin(LED_R1_GPIO_Port, LED_R1_Pin);
		Bootloader_JumpToSystem();
	} else if(score1 == score2) {
		int app = settings.app_to_choose_when_equal;
		settings.app_to_choose_when_equal = (settings.app_to_choose_when_equal + 1) % 2;
		Bootloader_SaveSettings();
		Bootloader_StartIWDG();
		Bootloader_JumpApp(app);
	} else {
		LL_GPIO_SetOutputPin(LED_G1_GPIO_Port, LED_G1_Pin);
		Bootloader_JumpApp(0);
	}

}
