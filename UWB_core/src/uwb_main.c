/*
 * uwb_main.c
 *
 *  Created on: 21.03.2018
 *      Author: KarolTrzcinski
 */

#include "mac/sync.h"

void desynchronize() {
  unsigned int seed = HAL_GetTick();
  for (int i = 0; i < rand_r(&seed) % 100; ++i) {
    port_watchdog_refresh();
    HAL_Delay(1);
  }
  port_watchdog_refresh();
}

void uwb_main() {
  settings_init();
  desynchronize();
  mac_init();

  while (1) {
    port_led_off(LED_STAT);
    port_led_on(LED_ERR);
  }
}
