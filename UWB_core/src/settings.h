/*
 * @file settings.h
 *
 *  Created on: 21.03.2018
 *      Author: Karol Trzcinski
 *
 * @brief definitions of settings structures
 *
 */
#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "gitversion.h"
#include "mac/mac_settings.h"
#include "mac/carry_settings.h"
#include "transceiver_settings.h"
#include "platform/ble_settings.h"
#include "ranging_settings.h"
#include "imu_settings.h"

/**
 * @brief generate hardware version description byte from major and minor
 *
 * It it possible to define up to 32 major firmware version and 8 minor.
 * @note Minor versions should be pin to pin compatible and run with same firmware.
 *
 */
#define H_VERSION_CALC(major, minor) ((major << 3) | (minor & 0x07))

/**
 * @brief calculate hardware major version from description byte
 *
 * It it possible to define up to 32 major firmware version and 8 minor.
 * @note Minor versions should be pin to pin compatible and run with same firmware.
 *
 */
#define H_MAJOR_CALC(hversion) ((hversion&0xFF)>>3)

/**
 * @brief generate current version number
 */
#define __H_VERSION__ H_VERSION_CALC(__H_MAJOR__, __H_MINOR__)

/**
 * @brief Internal One Time Programmable settings structure.
 *
 * This kind of settings is specially usable for hardware
 * and serial number description because it should be
 * constant value from production without possibility of
 * change.
 *
 * @note This structure is packed
 */
typedef struct
	__packed
	{
		uint64_t serial; ///< hardware serial number
		uint8_t h_major; ///< hardware major version number
		uint8_t h_minor; ///< hardware minor version number
	} settings_otp_t;

	/**
	 * @brief current firmware and hardware description structure
	 *
	 * Values from this struct should be modified only by firmware
	 * upgrade procedure.
	 *
	 * @note It is used also from bootloader to check firmware
	 *   and hardware compability between code versions.
	 *
	 * @note This structure is packed
	 */
	typedef struct
		__packed
		{
			uint32_t boot_reserved; ///< field reserved for bootloader
			uint8_t h_version; ///< hardware version from #H_VERSION_CALC
			uint8_t f_major; ///< firmware major version number
			uint16_t f_minor; ///< firmware minor version number
			uint64_t f_hash; ///< firmware hash version number
		} settings_version_t;

		/**
		 * @brief main setting container.
		 *
		 * This structure consist of settings structure
		 * from particular modules.
		 *
		 * @note #settings_version_t need to the first field in settings
		 *   because of reference from bootoader
		 *
		 * @note instance of this structure should be global and be located
		 *   in precisely defined location in non-volatile memory (FLASH)
		 *
		 */
		typedef struct {
			settings_version_t version;          ///< current firmware version
			transceiver_settings_t transceiver;  ///< transceiver settings
			mac_settings_t mac;          ///<  medium access control module settings
			carry_settings_t carry;      ///< carry protocol settings
			ble_settings_t ble;         ///< bluetooth low energy protocol settings
			ranging_settings_t ranging;  ///< ranging traces
      imu_settings_t imu;       ///< imu settings
		} settings_t;

		/**
		 * @brief default firmware and hardware version settings
		 *
		 */
#define VERSION_SETTINGS_DEF  \
  {                           \
  .boot_reserved = 0,         \
  .h_version = __H_VERSION__, \
  .f_major = __F_MAJOR__,     \
  .f_minor = __F_MINOR__,     \
  .f_hash = __F_HASH__,       \
  }

		/**
		 * @brief define default settings values
		 *
		 */
#define DEF_SETTINGS                                                          \
  {                                                                           \
    .version = VERSION_SETTINGS_DEF, .transceiver = TRANSCEIVER_SETTINGS_DEF, \
    .mac = MAC_SETTINGS_DEF, .carry = CARRY_SETTINGS_DEF,                     \
    .ble = BLE_SETTINGS_DEF, .ranging = RANGING_SETTINGS_DEF,				  \
    .imu = IMU_SETTINGS_DEF,                                                  \
  }

		/**
		 * @brief global instance of settings object
		 *
		 * In #SETTINGS_Init() or during startup values should be copied from
		 * non-volatile memory to this instance.
		 */
		extern settings_t settings;

		/**
		 * @brief global instance of #settings_otp_t object
		 *
		 * Pointed struct may depends on OTP memory status.
		 * When OTP is originally erased then pointed should be
		 * default otp struct, generated basing on device identifier.
		 */
		extern settings_otp_t const *settings_otp;

		/**
		 * @brief Initialize settings with default values.
		 *
		 * OTP settings can be initialized conditionally to
		 * current value of OTP address. Address from OTP will
		 * be used only if it is not filled with 0xFF.
		 *
		 */
		void SETTINGS_Init();

		/**
		 * @brief Save values from settings to flash startup values.
		 *
		 * This function feed watchdog and enter critical section.
		 *
		 * @return 0 when success
		 * @return 1 when erasing error
		 * @return 2 when writing error
		 *
		 */
		int SETTINGS_Save();

#endif
