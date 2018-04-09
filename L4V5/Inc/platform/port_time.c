/*
 * port_time.c
 *
 *  Created on: 22.03.2018
 *      Author: KarolTrzcinski
 */

#include "port.h"

#define LPTIM_SLEEP LPTIM1
#define LPTIM_SLOT LPTIM2

// define how many high resolution clock tick is in one us
#define PORT_TICKS_HR_PER_US 1000

void PORT_TimeInit() {
  // sleep timer
  // f = 32kHz / 32 = 1kHz
  // T = 1 ms
  LL_LPTIM_EnableIT_ARRM(LPTIM_SLEEP);
  LL_LPTIM_SetAutoReload(LPTIM_SLEEP, 1000);
  LL_LPTIM_Enable(LPTIM_SLEEP);

  // slot timer
  // f = 16MHz / 64 = 250kHz
  // T = 4 us
  LL_LPTIM_EnableIT_ARRM(LPTIM_SLOT);
  LL_LPTIM_SetAutoReload(LPTIM_SLOT, 1000);
  LL_LPTIM_Enable(LPTIM_SLOT);
}

void PORT_TimeStartTimers() {
  LL_LPTIM_StartCounter(LPTIM_SLEEP, LL_LPTIM_OPERATING_MODE_CONTINUOUS);
  LL_LPTIM_StartCounter(LPTIM_SLOT, LL_LPTIM_OPERATING_MODE_CONTINUOUS);
}

void PORT_SleepMs(unsigned int time_ms) {
  int end = time_ms + PORT_TickMs();
  while (end - (int)PORT_TickMs() > 0) {
    PORT_WatchdogRefresh();
  }
}

unsigned int PORT_TickMs() { return HAL_GetTick(); }

// get high resosolution clock tick

unsigned int PORT_TickHr() { return HAL_GetTick(); }

unsigned int PORT_FreqHr() { return HAL_RCC_GetSysClockFreq(); }

// update slot timer for one iteration, @us is us to the next IT
void PORT_SlotTimerSetUsLeft(uint32 us) {
  int delta =
      LL_LPTIM_GetAutoReload(LPTIM_SLOT) - LL_LPTIM_GetCounter(LPTIM_SLOT);
  if (us > 8 && delta > 5) {
    LPTIM2->CNT = us / 4;
  }
}

// set slot timer period
void PORT_SetSlotTimerPeriodUs(uint32 us) {
  // 1 tick = 4 us
  LL_LPTIM_SetAutoReload(LPTIM2, us / 4);
}
