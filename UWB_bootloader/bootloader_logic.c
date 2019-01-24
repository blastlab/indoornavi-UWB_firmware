/*
 * logic.c
 *
 *  Created on: 20.12.2018
 *      Author: KarolTrzcinski
 */

#include "bootloader.h"

#include <string.h>

const bl_set_t _flash_settings __attribute__((section (".settings")));
bl_set_t settings;


// array with application addresses
const void* bl_apps_addr[] = { (void*)APP1_ADDR, (void*)APP2_ADDR };

int settings_offset = 9;

int CheckHardwareVersions() {
	if (settings.my_hType == 0) { // situation from first run
		settings.my_hType = settings.stat[0].firmware_version->hType;
		return 1;
	}
	return 0;
}

int CheckPointerToAppSettings(app_set_t* pset, const char* app_start_addr) {
	const char* FLASH_END = (char*)FLASH_BASE + FLASH_SIZE;

// check pointer to firmware_version
	if (!((char*)FLASH_BASE < (char*)pset->firmware_version
	    && (char*)pset->firmware_version < FLASH_END)) {
		pset->firmware_version = (const settings_edit_t*)(app_start_addr + APP_TO_SETTINGS_OFFSET);
		return 1;
	}
	return 0;
}

int UpdateAppPassFailCounter(int previous_app) {
	app_set_t* pset = &settings.stat[0];
	int changes = 0;
	//read magic number
	if (Bootloader_ReadSpecialReg() == BOOTLOADER_MAGIC_NUMBER) {
		// correct firmware version
		if (pset[previous_app].pass_cnt < MAX_PASS_FAIL_COUNTER) {
			++pset[previous_app].pass_cnt;
			++changes;
		}
		if (pset[previous_app].fail_cnt > 0) {
			--pset[previous_app].fail_cnt;
			++changes;
		}
	} else {
		// bad firmware
		if (pset[previous_app].pass_cnt > 0) {
			--pset[previous_app].pass_cnt;
			++changes;
		}
		if (pset[previous_app].fail_cnt < MAX_PASS_FAIL_COUNTER) {
			++pset[previous_app].fail_cnt;
			++changes;
		}
	}
	return changes > 0;
}

void JumpApp(int index) {
	if(index == 0) {
		Bootloader_Led(LED_GREEN, LED_ON);
	} else if(index == 1) {
		Bootloader_Led(LED_RED, LED_ON);
	} else {
		Bootloader_Led(LED_GREEN, LED_ON);
		Bootloader_Led(LED_RED, LED_ON);
	}
	Bootloader_BkpSave(index + 1); // add 1 to ommit zero index
	if (index < sizeof(bl_apps_addr) / sizeof(*bl_apps_addr)) {
		Bootloader_JumpToAddr((long)bl_apps_addr[index]);
	}
}

static bool IsAnyApp(int app) {
	uint32_t* addr = (uint32_t*)bl_apps_addr[app];
	uint32_t ResetHandlerAddr = *(addr + 1);

	int badStackPonter = (((*(__IO uint32_t*)addr) & 0x2FFE0000) == 0x20000000) ? 0 : 1;
	int badResetHandlerAddr = ResetHandlerAddr < (uint32_t)addr
	    || (uint32_t)addr + APP_MAX_SIZE < ResetHandlerAddr;

	if (badStackPonter || badResetHandlerAddr) {
		return 0;
	} else {
		return 1;
	}
}

static bool IsNewFirmwareNumberInFlash(int app) {
	// check magic number at the end of a first page
	uint32_t* ptr = (uint32_t*)(APP1_ADDR + APP_MAX_SIZE * app + APP_TO_SETTINGS_OFFSET);
	uint32_t val = *ptr;
	return (val == BOOTLOADER_NEW_FIRMWARE_NUMBER);
}

static bool IsCorrectHardwareType(int app) {
	return settings.stat[app].firmware_version->hType == settings.my_hType;
}

int IsFirmwareNew(int app) {
	bool isAny = IsAnyApp(app);
	bool isNewInFlash = IsNewFirmwareNumberInFlash(app);
	bool isHTypeCorrect = IsCorrectHardwareType(app);
	if (isAny && isNewInFlash && isHTypeCorrect) {
		return 1;
	}
	return 0;
}

static int ScoreApplication(int i) {
	int score = 0;
	const app_set_t* pset = &settings.stat[i];

	if (IsAnyApp(i) != 0) {
		score = pset->pass_cnt - pset->fail_cnt;
		score *= score > 0; // trim lower bound to zero
		score += 1; // firmware is present
		score += pset->firmware_version->boot_reserved == BOOTLOADER_MAGIC_NUMBER;
		score += pset->firmware_version->hType == settings.my_hType;
	}

	return score;
}

void JumpToApi() {
	int score1, score2;

	score1 = ScoreApplication(0);
	score2 = ScoreApplication(1);

	if (score1 > score2) {
		JumpApp(0);
	} else if (score1 < score2) {
		JumpApp(1);
	} else if (score1 == 0 && score2 == 0) { // there is any api
		Bootloader_Led(LED_GREEN, LED_ON);
		Bootloader_Led(LED_RED, LED_ON);
		Bootloader_JumpToSystem();
	} else if (score1 == score2) {
		int app = settings.app_to_choose_when_equal;
		settings.app_to_choose_when_equal = (settings.app_to_choose_when_equal + 1) % 2;
		Bootloader_SaveSettings();
		Bootloader_StartIWDG();
		JumpApp(app);
	} else {
		Bootloader_Led(LED_GREEN, LED_ON);
		JumpApp(0);
	}
}

bool SomeAppWasRunnig() {
	bool some_app_was_run = *BOOTLOADER_BKP_REG == 1 || *BOOTLOADER_BKP_REG == 2;
	return some_app_was_run;
}

static bool IsTestProcJustFinished() {
	int previous_app = *BOOTLOADER_BKP_REG - 1;
	if (!SomeAppWasRunnig()) {
		return false;
	}
	const settings_edit_t* pset = settings.stat[previous_app].firmware_version;
	bool is_boot_reserved_clear = pset->boot_reserved != BOOTLOADER_MAGIC_NUMBER;
	bool all_pass_are_zero = !settings.stat[0].pass_cnt && !settings.stat[1].pass_cnt;
	bool all_fail_are_zero = !settings.stat[0].fail_cnt && !settings.stat[1].fail_cnt;
	return is_boot_reserved_clear && all_pass_are_zero && all_fail_are_zero;
}

void StartTest(int app) {
	// set each counters to 0
	for (int i = 0; i < 2; ++i) {
		settings.stat[i].fail_cnt = 0;
		settings.stat[i].pass_cnt = 0;
	}
	// prepare to run app next time
	Bootloader_WriteSpecialReg(0);			// only when new app appears
	Bootloader_BkpSave(0);
	Bootloader_SaveSettings();
	// start IWDG
	Bootloader_StartIWDG();
	JumpApp(app);
}

void Start(uint32_t reset_src) {
	int previous_app = *BOOTLOADER_BKP_REG - 1;
	uint8_t change_cnt = 0;

	// assert linker variable
	extern uint32_t PROG_FLASH_START, PROG_PAGE_SIZE;
	while ((uint32_t)(&PROG_FLASH_START) != FLASH_BASE) {
	};
	while ((uint32_t)(&PROG_PAGE_SIZE) != FLASH_PAGE_SIZE) {
	};

	// wczytaj zapisane ustawienia z FLASH do RAM
	memcpy(&settings, &_flash_settings, sizeof(settings));

	// set proper pointers in pset[i].settings
	for (int i = 0; i < 2; ++i) {
		change_cnt += CheckPointerToAppSettings(&settings.stat[i], bl_apps_addr[i]);
	}

	// if test procedure has just finished, then check app as correct or failed
	// (base on MAGIC_REG and MAGIC_NUMBER)
	// then mark them as old
	if (IsTestProcJustFinished()) {
		change_cnt += UpdateAppPassFailCounter(previous_app);
		Bootloader_MarkFirmwareAsOld(previous_app); // change only the flash data
		Bootloader_BkpSave(0);
	}

	// when it is a first run, then you should save correct hardware type
	change_cnt += CheckHardwareVersions();

	if (change_cnt > 0) {
		Bootloader_SaveSettings();
	}

	// and check if is new firmware, when it is true then settings will be saved and
	// new firmware will be launched.
	for (int i = 0; i < 2; ++i) {
		if (IsFirmwareNew(i)) {
			StartTest(i); // will save the settings and jump to app
		}
	}

	// software reset - run last App
	if (SomeAppWasRunnig() && IsSoftResetFromApp(reset_src)) {
		JumpApp(previous_app);
	}

	// when it's not a IsSoftResetFromApp()
	// and when it's not a IsTestProcJustFinished()
	// and when there is no IsFirmwareNew()
	// then choose the previous correctly working app
	JumpToApi();
}
