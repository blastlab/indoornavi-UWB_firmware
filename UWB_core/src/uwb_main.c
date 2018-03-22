/*
 * uwb_main.c
 *
 *  Created on: 21.03.2018
 *      Author: KarolTrzcinski
 */

#include "mac/mac.h"

void desynchronize()
{
  unsigned int seed = HAL_GetTick();
  port_sleep_ms(rand_r(&seed) % 100);
  port_watchdog_refresh();
}

void turnOff()
{
	// log turn off to host
	LOG_INF("turn off");

	// wait for packet transmission
  port_sleep_ms(100);

	transceiver_enter_deep_sleep();
	port_led_off(LED_R1);
	port_led_off(LED_G1);

	// 3 times blink leds
	for (int i = 0; i < 3; ++i) {
		port_sleep_ms(300);
		port_led_on(LED_R1);
		port_sleep_ms(300);
		port_led_off(LED_R1);
	}

	//disable IRQ
	__disable_irq();

	// wait forever
	// przeprowadz reset aby wylaczyc WWDG, a nastepnie
	// przejdz do trybu oszczednosci energii
	HAL_PWR_EnableBkUpAccess();
	BOOTLOADER_MAGIC_REG = BOOTLOADER_MAGIC_REG_GO_SLEEP;
	HAL_PWR_DisableBkUpAccess();
	port_reboot();
	while(1);
}

void enter_stop_mode()
{
	transceiver_enter_deep_sleep();
	while(1) {
		port_enter_stop_mode();
	}
}

void uwb_main() {
	unsigned int last_batt_measure_time = 0;

  spi_init();

	if(BOOTLOADER_MAGIC_REG == BOOTLOADER_MAGIC_REG_GO_SLEEP) {
		enter_stop_mode();
	}

	port_watchdog_init();
  settings_init();
  desynchronize();

  mac_init();
  carry_init();

  while (1) {
    port_led_off(LED_STAT);
    port_led_on(LED_ERR);

    if(port_tick_ms() - last_batt_measure_time > 5000) {
    	port_battery_measure();
    	last_batt_measure_time = port_tick_ms();
    }
  }
}
