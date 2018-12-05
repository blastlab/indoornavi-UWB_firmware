/*
 * port_time.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */

#include "port.h"
#include "mac.h"
#include "settings.h"
#include "nrf_soc.h"
#include "nrf_delay.h"
#include "nrfx_timer.h"
#include "nrfx_rtc.h"
#include "nrfx_clock.h"
#include "nrf_gpio.h"

static const nrfx_timer_t TIMER_SLOT = NRFX_TIMER_INSTANCE(1);
static const nrfx_rtc_t RTC = NRFX_RTC_INSTANCE(1);

static void rtc_handler(nrfx_rtc_int_type_t int_type) { }

struct {
		volatile uint32_t period;		// current timer's period (in ticks)
		volatile uint32_t buffer;		// buffer for stashing current period when offset is set
} slot_timer_s;

static void timer_slot_event_handler(nrf_timer_event_t event_type, void* p_context) {
	if(event_type == NRF_TIMER_EVENT_COMPARE0) {
		if(slot_timer_s.buffer != 0) {
			slot_timer_s.period = slot_timer_s.buffer;
			slot_timer_s.buffer = 0;
		}
		TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL0] += slot_timer_s.period;
		MAC_YourSlotIsr();
	}
}

void PORT_TimeInit() {
	nrfx_clock_hfclk_start();																// gives good slot timer synchronization
	nrfx_clock_lfclk_start();																// needed for RTC timer
	nrfx_rtc_config_t rtc_config = NRFX_RTC_DEFAULT_CONFIG;
	APP_ERROR_CHECK(nrfx_rtc_init(&RTC, &rtc_config, rtc_handler));
	nrfx_rtc_enable(&RTC);

	nrfx_timer_config_t timer_config = NRFX_TIMER_DEFAULT_CONFIG;
	APP_ERROR_CHECK(nrfx_timer_init(&TIMER_SLOT, &timer_config, timer_slot_event_handler));
	slot_timer_s.period = nrfx_timer_ms_to_ticks(&TIMER_SLOT, 100);
	slot_timer_s.buffer = 0;
	nrfx_timer_enable(&TIMER_SLOT);

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
BLE_CODE(
	if(settings.ble.is_enabled) {
		PORT_BleSetAdvData(settings.mac.addr, 0, 0);
		PORT_BleAdvStart();
	}
)
#if USE_SLOT_TIMER
	TIMER_SLOT.p_reg->CC[1] = 0;
	TIMER_SLOT.p_reg->TASKS_CAPTURE[NRF_TIMER_CC_CHANNEL1] = 1;
	nrfx_timer_compare(&TIMER_SLOT, NRF_TIMER_CC_CHANNEL0, slot_timer_s.period + TIMER_SLOT.p_reg->CC[1], true);
#endif
}

void PORT_SleepMs(time_ms_t time_ms) {
	nrf_delay_ms(time_ms);
}

time_ms_t PORT_TickMs() { return RTC.p_reg->COUNTER; }

// get high resosolution clock tick
unsigned int PORT_TickHr() { return DWT->CYCCNT; }

unsigned int PORT_TickHrToUs(unsigned int delta) {		// TODO implement this
	return 0;
}

// return current slot timer tick counter
uint32_t PORT_SlotTimerTickUs() {
#if USE_SLOT_TIMER
	TIMER_SLOT.p_reg->CC[1] = 0;
	TIMER_SLOT.p_reg->TASKS_CAPTURE[NRF_TIMER_CC_CHANNEL1] = 1;
	return slot_timer_s.period - (TIMER_SLOT.p_reg->CC[0] - TIMER_SLOT.p_reg->CC[1]);
#endif
	return 0;
}

// extend slot timer period for one iteration by delta_us
void PORT_SlotTimerSetUsOffset(int32 delta_us) {
#if USE_SLOT_TIMER
	// get actual timer count to CC[1]
	TIMER_SLOT.p_reg->TASKS_CAPTURE[NRF_TIMER_CC_CHANNEL1] = 1;
	// changing us to ticks
	delta_us = (delta_us < 0)
			? (int32_t)(-1*nrfx_timer_us_to_ticks(&TIMER_SLOT, (uint32_t)abs(delta_us)))
			: nrfx_timer_us_to_ticks(&TIMER_SLOT, delta_us);
	// when actual compare value + delta is more than actual count + 25 (as reserve)
	if(TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL0] + delta_us > TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL1] + 50) {
		slot_timer_s.buffer = slot_timer_s.period;
		slot_timer_s.period += delta_us;
		TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL0] += delta_us;
	}
#endif
}

// set slot timer period
void PORT_SetSlotTimerPeriodUs(uint32 us) {
#if USE_SLOT_TIMER
	// getting actual timer count
	TIMER_SLOT.p_reg->CC[1] = 0;
	TIMER_SLOT.p_reg->TASKS_CAPTURE[NRF_TIMER_CC_CHANNEL1] = 1;
	// format us to timer ticks, based on actual frequency
	slot_timer_s.period = nrfx_timer_us_to_ticks(&TIMER_SLOT, us);
	// adding new period to actual count (as clearing the timer after period change)
	TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL0] = TIMER_SLOT.p_reg->CC[1] + slot_timer_s.period;
#endif
}

void PORT_SetBeaconTimerPeriodMs(int ms) {
	//todo: TDOA beacon timer
}
