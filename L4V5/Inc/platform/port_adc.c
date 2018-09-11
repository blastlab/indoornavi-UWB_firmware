/*
 * port_battery.c
 *
 *  Created on: 22.03.2018
 *      Author: KarolTrzcinski
 */

#include "platform/port.h"
#include "settings.h"

extern ADC_HandleTypeDef hadc1; // in main.c

#define BAT_PMOS_ACTIVE_HIGH 0
#define ADC_HADC_VBAT hadc1
#define ADC_CH_VBAT ADC_CHANNEL_8
#define ADC_CH_HW	ADC_CHANNEL_6

// Internal voltage reference, address of parameter VREFINT_CAL:
// VrefInt ADC raw data acquired at temperature 30 DegC (tolerance: +-5 DegC),
// Vref+ = 3.0 V (tolerance: +-10 mV).
#ifndef VREFINT_CAL_ADDR
#define VREFINT_CAL_ADDR ((uint16_t *)(0x1FFF75AAU))
#endif

const float batterFilterCoeff = 0.5f;

static unsigned int _battery_mv = 0;

void PORT_AdcInit()
{
	HAL_ADCEx_Calibration_Start(&ADC_HADC_VBAT, ADC_SINGLE_ENDED);
	HAL_ADCEx_Calibration_GetValue(&ADC_HADC_VBAT, ADC_SINGLE_ENDED);
}

int PORT_AdcMeasure(uint32_t channel) {
	int adcInt;
	ADC_ChannelConfTypeDef ch;
	ch.Channel = channel;
	ch.Rank = ADC_REGULAR_RANK_1;
	ch.SamplingTime = ADC_SAMPLETIME_6CYCLES_5;
	ch.OffsetNumber = ADC_OFFSET_NONE;
	ch.SingleDiff = ADC_SINGLE_ENDED;
	ch.Offset = 0;
	PORT_ASSERT(HAL_ADC_ConfigChannel(&ADC_HADC_VBAT, &ch) == HAL_OK);
	PORT_ASSERT(HAL_ADC_Start(&ADC_HADC_VBAT) == HAL_OK);
	PORT_ASSERT(HAL_ADC_PollForConversion(&ADC_HADC_VBAT, 10) == HAL_OK);
	adcInt = HAL_ADC_GetValue(&ADC_HADC_VBAT);
	PORT_ASSERT(HAL_ADC_Stop(&ADC_HADC_VBAT) == HAL_OK);
	return adcInt;
}

// measure current battery voltage
void PORT_BatteryMeasure() {
  int adcInt, adcBat, nap;
  float VDDA;
// configure
#if BAT_PMOS_ACTIVE_HIGH
  HAL_GPIO_WritePin(VBAT_MOS_GPIO_Port, VBAT_MOS_Pin, GPIO_PIN_SET);
#else
  HAL_GPIO_WritePin(VBAT_MOS_GPIO_Port, VBAT_MOS_Pin, GPIO_PIN_RESET);
#endif
	adcInt = PORT_AdcMeasure(ADC_CHANNEL_VREFINT);
	adcBat = PORT_AdcMeasure(ADC_CH_VBAT);
#if BAT_PMOS_ACTIVE_HIGH
  HAL_GPIO_WritePin(VBAT_MOS_GPIO_Port, VBAT_MOS_Pin, GPIO_PIN_RESET);
#else
  HAL_GPIO_WritePin(VBAT_MOS_GPIO_Port, VBAT_MOS_Pin, GPIO_PIN_SET);
#endif

  // convert to mV
  VDDA = adcInt > 0 ? (3.0f * (*VREFINT_CAL_ADDR) / adcInt) : 0;
  nap = (int)(1.51f * VDDA * adcBat / 4.096f) +
        90; // +90mV jest sprawdzane empirycznie
  // uwb_log(LOG_INFO, "myNap:%d mV:%d", nap, (int)mV);

  if (nap < VDDA * 1000) {
    nap = (int)(VDDA * 1000);
  }
  // filter result
  if (_battery_mv < 1000) { // first measurement
  	_battery_mv = nap;
  } else {
    float a = batterFilterCoeff;
    PORT_ASSERT(0.f <= a && a < 1.f);
    _battery_mv = a * nap + (1 - a) * _battery_mv;
  }
}

// return last battery voltage in [mV]
int PORT_BatteryVoltage() { return _battery_mv; }

rtls_role PORT_GetHwRole() {
	const int MeasIterations = 4;
	uint32_t adcMeasure = 0;

	HAL_GPIO_WritePin(HW_PULL_GPIO_Port, HW_PULL_Pin, GPIO_PIN_SET);
	PORT_SleepMs(2);
	for (int i = 0; i < MeasIterations; ++i) {
		adcMeasure += PORT_AdcMeasure(ADC_CH_HW);
	}
	HAL_GPIO_WritePin(HW_PULL_GPIO_Port, HW_PULL_Pin, GPIO_PIN_RESET);

	adcMeasure /= MeasIterations;

	// 12-bit value (4096)
	// max voltage - tag
	// half voltage - anchor
	// min voltage - sink
	if (adcMeasure < 1000) {
		return RTLS_SINK;
	} else if (adcMeasure < 3000) {
		return RTLS_ANCHOR;
	} else {
		return RTLS_TAG;
	}
}
