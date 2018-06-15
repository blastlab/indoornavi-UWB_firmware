/*
 * port_battery.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */

#include "port.h"
#include "settings.h"

//extern ADC_HandleTypeDef hadc1; // in main.c TODO: implement adc handler

#define BAT_PMOS_ACTIVE_HIGH 0
//#define ADC_HADC_VBAT hadc1	TODO: implement adc handler

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

}

// measure current battery voltage
void PORT_BatteryMeasure() {

}

// return last battery voltage in [mV]
int PORT_BatteryVoltage() { return _battery_mv; }
