/*
 * uwb_main.c
 *
 *  Created on: 21.03.2018
 *      Author: KarolTrzcinski
 */

#include "mac/mac.h"
#include "prot/carry.h"

void Desynchronize()
{
	unsigned int seed = HAL_GetTick();
	PORT_SleepMs(rand_r(&seed) % 100);
	PORT_WatchdogRefresh();
}

void TurnOff()
{
	// log turn off to host
	LOG_INF("turn off");

	// wait for packet transmission
	PORT_SleepMs(100);

	TRANSCEIVER_EnterDeepSleep();
	PORT_LedOff(LED_R1);
	PORT_LedOff(LED_G1);

	// 3 times blink leds
	for (int i = 0; i < 3; ++i)
	{
		PORT_SleepMs(300);
		PORT_LedOn(LED_R1);
		PORT_SleepMs(300);
		PORT_LedOff(LED_R1);
	}

	//disable IRQ
	__disable_irq();

	// wait forever
	// przeprowadz reset aby wylaczyc WWDG, a nastepnie
	// przejdz do trybu oszczednosci energii
	HAL_PWR_EnableBkUpAccess();
	BOOTLOADER_MAGIC_REG = BOOTLOADER_MAGIC_REG_GO_SLEEP;
	HAL_PWR_DisableBkUpAccess();
	PORT_Reboot();
	while (1)
		;
}

void BatteryControl()
{
	static unsigned int last_batt_measure_time = 0;
	if (PORT_TickMs() - last_batt_measure_time > 5000)
	{
		PORT_BatteryMeasure();
		last_batt_measure_time = PORT_TickMs();

		if (2400 < PORT_BatteryVoltage() && PORT_BatteryVoltage() < 3100)
		{
			TurnOff();
		}
	}
}

void RangingControl()
{
}

void UwbMain()
{

	if (BOOTLOADER_MAGIC_REG == BOOTLOADER_MAGIC_REG_GO_SLEEP)
	{
		while (1)
		{
			PORT_EnterStopMode();
		}
	}

	SETTINGS_Init();
	Desynchronize();

	PORT_Init();
	MAC_Init();
	CARRY_Init();

	while (1)
	{
		PORT_LedOff(LED_STAT);
		PORT_LedOff(LED_ERR);
		BatteryControl();
		RangingControl();
	}
}
