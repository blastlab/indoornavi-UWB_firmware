/*
 * port_time.c
 *
 *  Created on: 22.03.2018
 *      Author: KarolTrzcinski
 */

#include "port.h"

void port_sleep_ms(unsigned int time_ms) {
  int end = time_ms + port_tick_ms();
  while (end - (int)port_tick_ms() > 0) {
    port_watchdog_refresh();
  }
}

unsigned int port_tick_ms() { return HAL_GetTick(); }

// get high resosolution clock tick
unsigned int port_tick_hr() { return 0; }

unsigned int port_freq_hr() { return HAL_RCC_GetSysClockFreq(); }
