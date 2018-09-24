/*
 * ble.c
 *
 *  Created on: 17 lip 2018
 *      Author: DawidPeplinski
 */
#include "port.h"
#include "FU.h"
#include "app_error.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"

#define APP_BLE_CONN_CFG_TAG            1                                  /**< A tag identifying the SoftDevice BLE configuration. */

volatile bool m_ble_radio_active;
void SWI1_IRQHandler(void)						// radio notifications
{
	m_ble_radio_active = !m_ble_radio_active;	// implementation copied from nordic's library
}

static void ble_stack_init(void)
{
    ret_code_t err_code;
    m_ble_radio_active = false;
    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);
}

void PORT_BleBeaconInit(void) {
	if(settings.mac.role != RTLS_SINK && settings.mac.role != RTLS_ANCHOR) {
		settings.ble.is_enabled = 0;
	}
	if(settings.ble.is_enabled) {
	    ble_stack_init();
//	    advertising_init();
//	    PORT_BleSetPower(settings.ble.tx_power);
	}
}
