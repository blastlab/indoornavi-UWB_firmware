/*
 * port_time.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */

#include "port.h"
#include "mac.h"
#include "nrf_delay.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"

//#define LPTIM_SLEEP LPTIM1 TODO: implement LPTIM_SLEEP handler
const nrf_drv_timer_t TIMER_SLOT = NRF_DRV_TIMER_INSTANCE(0);
const nrf_drv_rtc_t RTC = NRF_DRV_RTC_INSTANCE(0);

static void rtc_handler(nrf_drv_rtc_int_type_t int_type) {}

static void timer_slot_event_handler(nrf_timer_event_t event_type, void* p_context) {
	if(event_type == NRF_TIMER_EVENT_COMPARE0 || event_type == NRF_TIMER_EVENT_COMPARE1) {
		if(event_type == NRF_TIMER_EVENT_COMPARE1) {
			TIMER_SLOT.p_reg->TASKS_CLEAR = 1;
			TIMER_SLOT.p_reg->INTENCLR = (NRF_TIMER_INT_COMPARE0_MASK << NRF_TIMER_CC_CHANNEL1);
			TIMER_SLOT.p_reg->INTENSET = (NRF_TIMER_INT_COMPARE0_MASK << NRF_TIMER_CC_CHANNEL0);
			TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL1] = 0;
		}
		decaIrqStatus_t st = decamutexon();
		MAC_YourSlotIsr();
		decamutexoff(st);
	}
}

void PORT_TimeInit() {
    APP_ERROR_CHECK(nrf_drv_clock_init());
    nrf_drv_clock_lfclk_request(NULL);
    nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
    APP_ERROR_CHECK(nrf_drv_rtc_init(&RTC, &config, rtc_handler));
    nrf_drv_rtc_enable(&RTC);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;					// TIMER: f = 1 MHz; T = 1 us
    APP_ERROR_CHECK(nrf_drv_timer_init(&TIMER_SLOT, &timer_cfg, timer_slot_event_handler));
    nrf_drv_timer_extended_compare(
         &TIMER_SLOT, NRF_TIMER_CC_CHANNEL0, nrf_drv_timer_ms_to_ticks(&TIMER_SLOT, 100), NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
}

void PORT_TimeStartTimers() {
	nrf_drv_timer_enable(&TIMER_SLOT);
}

void PORT_SleepMs(unsigned int time_ms) {
	nrf_delay_ms(time_ms);
}

unsigned int PORT_TickMs() { return RTC.p_reg->COUNTER; }

// get high resosolution clock tick
unsigned int PORT_TickHr() { return DWT->CYCCNT; }

unsigned int PORT_TickHrToUs(unsigned int delta) {		// TODO implement this
	return 0;
}

// update slot timer for one iteration, @us is us to the next IT
void PORT_SlotTimerSetUsLeft(uint32 us) {
	TIMER_SLOT.p_reg->TASKS_CAPTURE[NRF_TIMER_CC_CHANNEL1] = 1;
	int delta = TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL0] - TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL1];
	if (us > 8 && delta > 8) {
		TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL1] = TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL1] + nrf_drv_timer_us_to_ticks(&TIMER_SLOT, us);
		TIMER_SLOT.p_reg->INTENCLR = (NRF_TIMER_INT_COMPARE0_MASK << NRF_TIMER_CC_CHANNEL0);
		TIMER_SLOT.p_reg->INTENSET = (NRF_TIMER_INT_COMPARE0_MASK << NRF_TIMER_CC_CHANNEL1);
	}
}

// set slot timer period
void PORT_SetSlotTimerPeriodUs(uint32 us) {
	TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL0] = nrf_drv_timer_us_to_ticks(&TIMER_SLOT, us);
	TIMER_SLOT.p_reg->TASKS_CLEAR = 1;
}
