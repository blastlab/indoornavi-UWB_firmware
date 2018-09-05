/*
 * ble_settings.h
 *
 *  Created on: 3 sie 2018
 *      Author: DawidPeplinski
 */

#ifndef PLATFORM_BLE_SETTINGS_H_
#define PLATFORM_BLE_SETTINGS_H_

typedef struct {
	int8_t tx_power;
	uint8_t is_enabled;
} ble_settings_t;

#define BLE_SETTINGS_DEF { 		\
		.tx_power = 0,			\
		.is_enabled = USE_BLE,	\
	}							\


#endif /* PLATFORM_BLE_SETTINGS_H_ */
