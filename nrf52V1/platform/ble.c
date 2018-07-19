/*
 * ble.c
 *
 *  Created on: 17 lip 2018
 *      Author: DawidPeplinski
 */
#include "port.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "ble_advdata.h"

#define APP_BLE_CONN_CFG_TAG            1                                 /* A tag identifying the SoftDevice BLE configuration. */
#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(100, UNIT_0_625_MS) /* The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s). */
#define APP_ADV_DATA_LENGTH             0x15                              /* Length of manufacturer specific data in the advertisement. */
#define APP_DEVICE_TYPE                 0x02                              /* 0x02 refers to Beacon. */
#define APP_MEASURED_RSSI               0xC3                              /* The Beacon's measured RSSI at 1 meter distance in dBm. */
#define APP_COMPANY_IDENTIFIER          0x4C, 0x00                        /* Apple's company identifier for iOS devices */
#define APP_MAJOR_VALUE                 0x01, 0x02                        /* Major value used to identify Beacons. */
#define APP_MINOR_VALUE                 0x03, 0x04                        /* Minor value used to identify Beacons. */
#define APP_BEACON_UUID 				0x30, 0xfd, 0x7d, 0x40, \
										0x2e, 0xdc, 0x4d, 0x83, \
										0x9d, 0x47, 0xd8, 0x8a, \
										0xa7, 0xe0, 0x49, 0x2a 			  /* Full UUID: 30fd7d40-2edc-4d83-9d47-d88aa7e0492a */
#define DEAD_BEEF                       0xDEADBEEF                        /* Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define MANUFACTURER_DATA \
    APP_COMPANY_IDENTIFIER, \
	APP_DEVICE_TYPE,      			/* Manufacturer specific information. Specifies the device type in this implementation.	*/ \
	APP_ADV_DATA_LENGTH,			/* Manufacturer specific information. Specifies the length of the manufacturer specific data in this implementation. */ \
    APP_BEACON_UUID,     			/* 128 bit UUID value. */ \
    APP_MAJOR_VALUE,     			/* Major arbitrary value that can be used to distinguish between Beacons. */ \
    APP_MINOR_VALUE,     			/* Minor arbitrary value that can be used to distinguish between Beacons. */ \
    APP_MEASURED_RSSI    			/* Manufacturer specific information. The Beacon's measured TX power in this implementation. */


static ble_gap_adv_params_t m_adv_params;                                 /**< Parameters to be passed to the stack when starting advertising. */

#define ADV_DATA_LENGTH		0x1E
static uint8_t adv_data[ADV_DATA_LENGTH] = {
		0x02,											// size of block
		0x01,											// type of data (flags)
		BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,	// data

		0x1A,
		0xFF,											// Manufacturer Specific Data
		MANUFACTURER_DATA
};

#define SCRP_DATA_LENGTH 0x18
static uint8_t scrp_data[SCRP_DATA_LENGTH] = {
		0x05,
		0x09,											// Complete Local Name
		'N', 'a', 'v', 'i',

		0x11,
		0x07,											// Complete List of 128-bit Service Class UUIDs
		APP_BEACON_UUID,
};

void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name) {
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


void PORT_BleSetAdvData(uint16_t maj_val, uint16_t min_val) {
	adv_data[25] = (0xFF00 & maj_val) >> 8;
	adv_data[26] = 0x00FF & maj_val;
	adv_data[27] = (0xFF00 & min_val) >> 8;
	adv_data[28] = 0x00FF & min_val;
	sd_ble_gap_adv_data_set((uint8_t const *)&adv_data, ADV_DATA_LENGTH, (uint8_t const *)&scrp_data, SCRP_DATA_LENGTH);
}

static void advertising_init(void) {
	PORT_BleSetAdvData(0x0001, 0x0002);

    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));
    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_SCAN_IND;
    m_adv_params.p_peer_addr = NULL;    // Undirected advertisement.
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = NON_CONNECTABLE_ADV_INTERVAL;
    m_adv_params.timeout     = 0;       // Never time out.
}

void PORT_BleAdvStart(void) {
    APP_ERROR_CHECK(sd_ble_gap_adv_start(&m_adv_params, APP_BLE_CONN_CFG_TAG));
}

static void ble_stack_init(void) {
    APP_ERROR_CHECK(nrf_sdh_enable_request());

    uint32_t ram_start = 0;
    APP_ERROR_CHECK(nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start));

    APP_ERROR_CHECK(nrf_sdh_ble_enable(&ram_start));
}

void PORT_BleBeaconStart(void) {
    ble_stack_init();
    advertising_init();
}
