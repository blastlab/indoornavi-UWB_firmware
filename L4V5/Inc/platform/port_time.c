/*
 * port_time.c
 *
 *  Created on: 22.03.2018
 *      Author: KarolTrzcinski
 */

#include "port.h"

#define LPTIM_SLEEP LPTIM1
#define PTIM_SLOT TIM2

void PORT_TimeInit() {
  // sleep timer
  // f = 32kHz / 32 = 1kHz
  // T = 1 ms
  LL_LPTIM_EnableIT_ARRM(LPTIM_SLEEP);
  LL_LPTIM_SetAutoReload(LPTIM_SLEEP, 1000);
  LL_LPTIM_Enable(LPTIM_SLEEP);

  // slot timer
  // f = 40 MHz / 40 = 1MHz
  // T = 1 us
  // Tmax = more than 1h but only half of that is usable
  LL_TIM_SetPrescaler(PTIM_SLOT, 40);
  LL_TIM_SetAutoReload(PTIM_SLOT, UINT32_MAX);
  LL_TIM_SetCounterMode(PTIM_SLOT, LL_TIM_COUNTERMODE_DOWN);
  LL_TIM_EnableIT_UPDATE(PTIM_SLOT);

  // HR timer
  // enable Debug Timer for rtls processing time measurement
#if defined(__CORTEX_M) && DBG
  // Disable TRC
  CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; // ~0x01000000;
  // Enable TRC */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // 0x01000000;
  // Disable clock cycle counter
  DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk; //~0x00000001;
  // Enable  clock cycle counter
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; // 0x00000001;
  // Reset the clock cycle counter value */
  DWT->CYCCNT = 0;
  // 3 NO OPERATION instructions
  __ASM volatile("NOP");
  __ASM volatile("NOP");
  __ASM volatile("NOP");
#endif
}

void PORT_TimeStartTimers() {
  LL_LPTIM_StartCounter(LPTIM_SLEEP, LL_LPTIM_OPERATING_MODE_CONTINUOUS);
  LL_TIM_EnableCounter(PTIM_SLOT);
}

void PORT_SleepMs(unsigned int time_ms) {
  int end = time_ms + PORT_TickMs();
  while (end - (int)PORT_TickMs() > 0) {
    PORT_WatchdogRefresh();
  }
}

unsigned int PORT_TickMs() { return HAL_GetTick(); }

// get high resosolution clock tick
unsigned int PORT_TickHr() { return DWT->CYCCNT; }

unsigned int PORT_TickHrToUs(unsigned int delta) {
	return (uint64_t)(delta) * 1e6 / HAL_RCC_GetSysClockFreq();
 }

// return current slot timer tick counter
uint32_t PORT_SlotTimerTick() {
	return LL_TIM_GetCounter(PTIM_SLOT);
}

// extend slot timer period for one iteration by delta_us
void PORT_SlotTimerSetUsOffset(int32 delta_us) {
  // change value only if you have enough time to do that
  if (LL_TIM_GetCounter(PTIM_SLOT) > 50 - delta_us) {
	  uint32_t tim_cnt = LL_TIM_GetCounter(PTIM_SLOT);
	  LL_TIM_SetCounter(PTIM_SLOT, tim_cnt + delta_us);
  }
}

// set slot timer period
void PORT_SetSlotTimerPeriodUs(uint32 us) {
  // 1 tick = 1 us
  LL_TIM_SetAutoReload(PTIM_SLOT, us);
}
