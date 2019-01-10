/*
 * port_adc.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */

#include "port.h"
#include "nrfx_saadc.h"
#include "nrf_gpio.h"
#include "settings.h"

// Vref+ = 3.6 V (0.6V /(1/6)) = ref_internal/gain
#define VREF 3.6

const float batterFilterCoeff = 0.5f;
static unsigned int _battery_mv = 0;

static volatile bool saadc_ready;
void saadc_callback(nrfx_saadc_evt_t const * p_event) {
	if(p_event->type == NRFX_SAADC_EVT_CALIBRATEDONE) {
		saadc_ready = true;
	}
}

static void waitForVcc() {
	nrf_saadc_channel_config_t check_vcc_config =
	NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_VDD);
	APP_ERROR_CHECK(nrfx_saadc_channel_init(2, &check_vcc_config));
	float vcc = 0;
	do {
		nrf_saadc_value_t raw_voltage;
		nrfx_saadc_sample_convert(2, &raw_voltage);
		vcc = (float)raw_voltage / 1024.0 * VREF;
	} while (vcc < 2.8);
	nrfx_saadc_channel_uninit(2);
}

void PORT_BatteryInit() {
#if HW_TYPE_PULL || BATT_ADC_TRIG_PIN
	nrfx_saadc_config_t saadc_conf = NRFX_SAADC_DEFAULT_CONFIG;
	nrfx_saadc_init(&saadc_conf, saadc_callback);
	waitForVcc();
	saadc_ready = false;
	APP_ERROR_CHECK(nrfx_saadc_calibrate_offset());
	while(!saadc_ready) PORT_WatchdogRefresh();
#endif
#if BATT_ADC_TRIG_PIN
    nrf_saadc_channel_config_t batt_channel_config =
    		NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN6);
    APP_ERROR_CHECK(nrfx_saadc_channel_init(0, &batt_channel_config));
#endif
#if HW_TYPE_PULL
    nrf_saadc_channel_config_t hw_type_channel_config =
            NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN7);
	APP_ERROR_CHECK(nrfx_saadc_channel_init(1, &hw_type_channel_config));
#endif
}

// measure current battery voltage
void PORT_BatteryMeasure() {
#if BATT_ADC_TRIG_PIN
	int16_t voltage = 0;
	nrf_gpio_cfg_input(BATT_ADC_TRIG_PIN, NRF_GPIO_PIN_PULLDOWN);
	PORT_SleepMs(20);
	float sum = 0;
	for(uint8_t i = 0; i < 4; i++) {
		nrfx_saadc_sample_convert(0, &voltage);
		sum += voltage;
	}
	nrf_gpio_cfg_input(BATT_ADC_TRIG_PIN, NRF_GPIO_PIN_PULLUP);
	sum = sum/4.0/1024.0*VREF*1000;		// voltage on pin (mV)
	sum = sum*15.1/10.0;				// voltage on battery (before voltage divider)

	// filter result
	if (_battery_mv < 1000) { // first measurement
		_battery_mv = sum;
	} else {
		float a = batterFilterCoeff;
		PORT_ASSERT(0.f <= a && a < 1.f);
		_battery_mv = a * sum + (1 - a) * _battery_mv;
	}
#endif
}

// return last battery voltage in [mV]
int PORT_BatteryVoltage() { return _battery_mv; }


rtls_role PORT_GetHwRole() {
#if ETH_SINK_PCB
	return RTLS_SINK;
#endif
#if HW_TYPE_PULL
	int16_t voltage = 0;
	nrf_gpio_cfg_input(HW_TYPE_PULL, NRF_GPIO_PIN_PULLUP);
	PORT_SleepMs(5);
	PORT_WatchdogRefresh();
	int32_t sum = 0;
	for(uint8_t i = 0; i < 4; i++) {
		nrfx_saadc_sample_convert(1, &voltage);
		sum += voltage;
	}
	nrf_gpio_cfg_input(HW_TYPE_PULL, NRF_GPIO_PIN_PULLDOWN);
	sum = sum/4.0/1024.0*VREF*1000;		// voltage on pin (mV)
	if(sum > 2400) {
		return RTLS_TAG;
	}
	else if(sum > 800) {
		return RTLS_ANCHOR;
	}
	else {
		return RTLS_SINK;
	}
#endif
	return RTLS_DEFAULT;
}
