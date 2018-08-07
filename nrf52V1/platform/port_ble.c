/*
 * ble.c
 *
 *  Created on: 17 lip 2018
 *      Author: DawidPeplinski
 */
#include "port.h"
#include "FU.h"
#include "nrf_sdh.h"
#include "nrf_sdm.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "ble_advdata.h"
#include "toa.h"

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

#define APP_UWB_DATA	 				0x00, 0x00, 0x00, 0x00, \
										0x00, 0x00, 0x00, 0x00, \
										0x00, 0x00, 0x00, 0x00, \
										0xBE, 0xCA, 0x19, 0x95
#define DEAD_BEEF                       0xDEADBEEF                        /* Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#define MANUFACTURER_DATA \
    APP_COMPANY_IDENTIFIER, \
	APP_DEVICE_TYPE,      			/* Manufacturer specific information. Specifies the device type in this implementation.	*/ \
	APP_ADV_DATA_LENGTH,			/* Manufacturer specific information. Specifies the length of the manufacturer specific data in this implementation. */ \
	APP_BEACON_UUID,     			/* 128 bit UUID data. */ \
    APP_MAJOR_VALUE,     			/* Major arbitrary value that can be used to distinguish between Beacons. */ \
    APP_MINOR_VALUE,     			/* Minor arbitrary value that can be used to distinguish between Beacons. */ \
    APP_MEASURED_RSSI    			/* Manufacturer specific information. The Beacon's measured TX power in this implementation. */

#define ADV_DATA_LENGTH		0x1E
#define SCRP_DATA_LENGTH 	0x18

static uint8_t adv_data[ADV_DATA_LENGTH] = {
		0x02,											// size of block
		0x01,											// type of data (flags)
		BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,	// data

		0x1A,
		0xFF,											// Manufacturer Specific Data
		MANUFACTURER_DATA
};

static uint8_t scrp_data[SCRP_DATA_LENGTH] = {
		0x05,
		0x09,											// Complete Local Name
		'N', 'a', 'v', 'i',

		0x11,
		0x07,											// Complete List of 128-bit Service Class UUIDs
		APP_BEACON_UUID,
};

static ble_gap_adv_params_t m_adv_params;                                 /**< Parameters to be passed to the stack when starting advertising. */
uint8_t APP_UWB_BLE_DATA[16];

extern void soc_evt_handler(uint32_t evt_id, void * p_context);

void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name) {
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

void PORT_BleSetAdvData(uint16_t maj_val, uint16_t min_val) {
	if(maj_val) {
		adv_data[25] = (0xFF00 & maj_val) >> 8;
		adv_data[26] = 0x00FF & maj_val;
	}
	if(min_val) {
		adv_data[27] = (0xFF00 & min_val) >> 8;
		adv_data[28] = 0x00FF & min_val;
	}
	if(settings.ble.is_enabled) {
		sd_ble_gap_adv_data_set((uint8_t const *)&adv_data, ADV_DATA_LENGTH, (uint8_t const *)&scrp_data, SCRP_DATA_LENGTH);
	}
}

void big_to_little(uint8_t *meas_addr) {
	uint8_t buf = 0;
	for(uint8_t i = 0; i < sizeof(measure_t); i+=2) {
		buf = meas_addr[i];
		meas_addr[i] = meas_addr[i + 1];
		meas_addr[i + 1] = buf;
	}
}

void PORT_SetUwbMeasuresAdv(uint8_t *meas_addr) {
//	big_to_little(meas_addr);
	memcpy(&adv_data[9], meas_addr, sizeof(measure_t));
	PORT_BleSetAdvData(0, 0);
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
	if(settings.ble.is_enabled) {
		APP_ERROR_CHECK(sd_ble_gap_adv_start(&m_adv_params, APP_BLE_CONN_CFG_TAG));
	}
}

void PORT_BleAdvStop(void) {
	if(settings.ble.is_enabled) {
		APP_ERROR_CHECK(sd_ble_gap_adv_stop());
	}
}

void PORT_BleSetPower(int8_t power) {
	settings.ble.tx_power = power;
	if(settings.ble.is_enabled) {
		APP_ERROR_CHECK(sd_ble_gap_tx_power_set(settings.ble.tx_power));
	}
}

static void ble_stack_init(void) {
	uint32_t ram_start = 0;
	APP_ERROR_CHECK(nrf_sdh_enable_request());
    APP_ERROR_CHECK(nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start));
    APP_ERROR_CHECK(nrf_sdh_ble_enable(&ram_start));
    NRF_SDH_SOC_OBSERVER(m_soc_observer, NRF_SDH_SOC_STACK_OBSERVER_PRIO, soc_evt_handler, NULL);
}

void PORT_BleBeaconInit(void) {
	if(settings.mac.role != RTLS_SINK && settings.mac.role != RTLS_ANCHOR) {
		settings.ble.is_enabled = 0;
	}
	if(settings.ble.is_enabled) {
	    ble_stack_init();
	    PORT_BleSetPower(settings.ble.tx_power);
	    advertising_init();
	}
    sd_softdevice_vector_table_base_set((uint32_t)(FU_GetCurrentFlashBase()));
}
