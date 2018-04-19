/*
 * port_battery.c
 *
 *  Created on: 22.03.2018
 *      Author: KarolTrzcinski
 */

#include "port.h"
#include "settings.h"

extern ADC_HandleTypeDef hadc1; // in main.c

#define BAT_PMOS_ACTIVE_HIGH 0
#define ADC_HADC_VBAT hadc1

// Internal voltage reference, address of parameter VREFINT_CAL:
// VrefInt ADC raw data acquired at temperature 30 DegC (tolerance: +-5 DegC),
// Vref+ = 3.0 V (tolerance: +-10 mV).
#ifndef VREFINT_CAL_ADDR
#define VREFINT_CAL_ADDR ((uint16_t *)(0x1FFF75AAU))
#endif

const float batterFilterCoeff = 0.5f;

static unsigned int _battery_mv = 0;

void PORT_BatteryInit()
{
	HAL_ADCEx_Calibration_Start(&ADC_HADC_VBAT, ADC_SINGLE_ENDED);
	HAL_ADCEx_Calibration_GetValue(&ADC_HADC_VBAT, ADC_SINGLE_ENDED);
}

// measure current battery voltage
void PORT_BatteryMeasure() {
  int adcInt, adcBat, nap;
  float VDDA;
  ADC_ChannelConfTypeDef ch;
  ch.Channel = ADC_CHANNEL_VREFINT;
  ch.Rank = 1;
  ch.SamplingTime = ADC_SAMPLETIME_92CYCLES_5;
// configure
#if BAT_PMOS_ACTIVE_HIGH
  HAL_GPIO_WritePin(VBAT_MOS_GPIO_Port, VBAT_MOS_Pin, GPIO_PIN_SET);
#else
  HAL_GPIO_WritePin(VBAT_MOS_GPIO_Port, VBAT_MOS_Pin, GPIO_PIN_RESET);
#endif
  HAL_ADC_Stop(&ADC_HADC_VBAT); // raczej niepotrzebne, testy do HardFault
  HAL_ADC_ConfigChannel(&ADC_HADC_VBAT, &ch); //todo: HardFault
  HAL_ADC_Start(&ADC_HADC_VBAT);
  // it is also delay
  if (HAL_ADC_PollForConversion(&ADC_HADC_VBAT, 10) == HAL_OK) {
    adcInt = HAL_ADC_GetValue(&ADC_HADC_VBAT);
  } else {
    adcInt = 0;
  }
  HAL_ADC_Stop(&ADC_HADC_VBAT);
  ch.Channel = ADC_CHANNEL_VBAT;
  HAL_ADC_ConfigChannel(&ADC_HADC_VBAT, &ch);
  HAL_ADC_Start(&ADC_HADC_VBAT);
  if (HAL_ADC_PollForConversion(&ADC_HADC_VBAT, 10) == HAL_OK) {
    adcBat = HAL_ADC_GetValue(&ADC_HADC_VBAT);
  } else {
    adcBat = 0;
  }
  HAL_ADC_Stop(&ADC_HADC_VBAT);
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
