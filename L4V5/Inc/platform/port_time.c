/*
 * port_time.c
 *
 *  Created on: 22.03.2018
 *      Author: KarolTrzcinski
 */

#include "platform/port.h"

#define LPTIM_BEACON LPTIM1
#define PTIM_SLOT TIM2

void PORT_TimeInit() {
	// sleep timer
	// f = 32kHz / 128 = 250Hz
	// T = 4 ms
	LL_LPTIM_EnableIT_ARRM(LPTIM_BEACON);
	LL_LPTIM_SetAutoReload(LPTIM_BEACON, 1000);
	LL_LPTIM_Enable(LPTIM_BEACON);

	// slot timer
	// f = 40 MHz / 40 = 1MHz
	// T = 1 us
	// Tmax = more than 1h but only half of that is usable
#if USE_SLOT_TIMER
	LL_TIM_SetPrescaler(PTIM_SLOT, 40);
	LL_TIM_SetAutoReload(PTIM_SLOT, UINT32_MAX);
	LL_TIM_SetCounterMode(PTIM_SLOT, LL_TIM_COUNTERMODE_DOWN);
	LL_TIM_EnableIT_UPDATE(PTIM_SLOT);
#endif

	// HR timer
	// enable Debug Timer for rtls processing time measurement
#if defined(__CORTEX_M) && DBG
	// Disable TRC
	CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;// ~0x01000000;
	// Enable TRC */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;// 0x01000000;
	// Disable clock cycle counter
	DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;//~0x00000001;
	// Enable  clock cycle counter
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;// 0x00000001;
	// Reset the clock cycle counter value */
	DWT->CYCCNT = 0;
	// 3 NO OPERATION instructions
	__ASM volatile("NOP");
	__ASM volatile("NOP");
	__ASM volatile("NOP");
#endif
}

void PORT_TimeStartTimers() {
	LL_TIM_EnableCounter(PTIM_SLOT);
}

void PORT_SleepMs(time_ms_t time_ms) {
	int end = time_ms + PORT_TickMs();
	while (end - (int)PORT_TickMs() > 0) {
		PORT_WatchdogRefresh();
	}
}

time_ms_t PORT_TickMs() {
	return HAL_GetTick();
}

// get high resosolution clock tick
unsigned int PORT_TickHr() {
	return DWT->CYCCNT;
}

unsigned int PORT_TickHrToUs(unsigned int delta) {
	return (uint64_t)(delta) * 1e6 / HAL_RCC_GetSysClockFreq();
}

// return current slot timer tick counter
uint32_t PORT_SlotTimerTickUs() {
	return LL_TIM_GetAutoReload(PTIM_SLOT) - LL_TIM_GetCounter(PTIM_SLOT);
}

// extend slot timer period for one iteration by delta_us
void PORT_SlotTimerSetUsOffset(int32 delta_us) {
	// change value only if you have enough time to do that
	if (LL_TIM_GetCounter(PTIM_SLOT) > 50 - delta_us) {
		uint32_t tim_cnt = LL_TIM_GetCounter(PTIM_SLOT);
		LL_TIM_SetCounter(PTIM_SLOT, tim_cnt + delta_us);
		if (LL_TIM_GetCounter(PTIM_SLOT) > LL_TIM_GetAutoReload(PTIM_SLOT)) {
			LL_TIM_SetCounter(PTIM_SLOT, 0);
			LL_TIM_GenerateEvent_UPDATE(PTIM_SLOT);
		}
	}
}

// set slot timer period
void PORT_SetSlotTimerPeriodUs(uint32 us) {
	// 1 tick = 1 us
	LL_TIM_SetAutoReload(PTIM_SLOT, us);
}

static int _PORT_InitRTC() {
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
	LL_PWR_EnableBkUpAccess();

	LL_RCC_LSI_Enable();

	while (LL_RCC_LSI_IsReady() != 1) {
	}
	LL_RCC_ForceBackupDomainReset();
	LL_RCC_ReleaseBackupDomainReset();
	LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSI);

	LL_RCC_EnableRTC();

	LL_RTC_InitTypeDef RTC_InitStruct;
	RTC_InitStruct.HourFormat = LL_RTC_HOURFORMAT_24HOUR;
	RTC_InitStruct.AsynchPrescaler = 127;
	RTC_InitStruct.SynchPrescaler = 24;
	LL_RTC_Init(RTC, &RTC_InitStruct); // 32e3 / 128 / 25 = 10 Hz
	int resolution_ms = 100;

	return resolution_ms;
}

void PORT_SetBeaconTimerPeriodMs(int time_ms) {
//	// start RTC init
	int resolution_ms = _PORT_InitRTC();

	// disable RTC write protection
	LL_RTC_DisableWriteProtection(RTC);

	// disable wake up timer to modify it
	LL_RTC_WAKEUP_Disable(RTC);
	while (LL_RTC_IsActiveFlag_WUTW(RTC) != 1) {
	}

	// set clock period
	LL_RTC_WAKEUP_SetAutoReload(RTC, time_ms / resolution_ms);
	LL_RTC_WAKEUP_SetClock(RTC, LL_RTC_WAKEUPCLOCK_CKSPRE);

	// clear IRQ flags end connect interrupts
	LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_20);
	LL_RTC_ClearFlag_WUT(RTC);
	LL_RTC_EnableIT_WUT(RTC);

	// enable WUT
	LL_RTC_WAKEUP_Enable(RTC);

	// enable RTC registers write protection
	LL_RTC_EnableWriteProtection(RTC);

	// enable RTC WakeUp timer interrupt
	LL_PWR_EnableInternWU();
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_20);
	LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_20);
	NVIC_EnableIRQ(RTC_WKUP_IRQn); // Configure NVIC for RTC
	NVIC_SetPriority(RTC_WKUP_IRQn, 2); // Set priority for RTC (=2, low priority)
}
