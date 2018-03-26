/*
 * port_time.c
 *
 *  Created on: 22.03.2018
 *      Author: KarolTrzcinski
 */

#include "port.h"

// define how many high resolution clock tick is in one us
#define PORT_TICKS_HR_PER_US 1000

void PORT_SleepMs(unsigned int time_ms) {
  int end = time_ms + PORT_TickMs();
  while (end - (int)PORT_TickMs() > 0) {
    PORT_WatchdogRefresh();
  }
}

unsigned int PORT_TickMs() { return HAL_GetTick(); }

// get high resosolution clock tick
unsigned int PORT_TickHr() { return 0; }

unsigned int PORT_FreqHr() { return HAL_RCC_GetSysClockFreq(); }
