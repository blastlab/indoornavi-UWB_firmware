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
#include "nrfx_clock.h"
#include "nrf_gpio.h"

static const nrfx_timer_t GENERAL_TIMER = NRFX_TIMER_INSTANCE(1);

struct {
		volatile uint32_t period;		// current timer's period (in ticks)
		volatile uint32_t buffer;		// buffer for stashing current period when offset is set
} slot_timer_s;

struct {
		volatile uint32_t period;
		volatile uint32_t tick_ms;
} tick_timer_s;

static void general_timer_event_handler(nrf_timer_event_t event_type, void* p_context) {
	if(event_type == NRF_TIMER_EVENT_COMPARE0) {
		if(slot_timer_s.buffer != 0) {
			slot_timer_s.period = slot_timer_s.buffer;
			slot_timer_s.buffer = 0;
		}
		GENERAL_TIMER.p_reg->CC[NRF_TIMER_CC_CHANNEL0] += slot_timer_s.period;
		MAC_YourSlotIsr();
	}
	if(event_type == NRF_TIMER_EVENT_COMPARE2) {
		GENERAL_TIMER.p_reg->CC[NRF_TIMER_CC_CHANNEL2] += tick_timer_s.period;
		tick_timer_s.tick_ms++;
	}
}

void PORT_TimeInit() {
	nrfx_clock_hfclk_start();																// gives good slot timer synchronization
	nrfx_timer_config_t timer_config = NRFX_TIMER_DEFAULT_CONFIG;
	nrfx_timer_init(&GENERAL_TIMER, &timer_config, general_timer_event_handler);
	slot_timer_s.period = nrfx_timer_ms_to_ticks(&GENERAL_TIMER, settings.mac.slots_sum_time_us);
	slot_timer_s.buffer = 0;
	tick_timer_s.period = nrfx_timer_us_to_ticks(&GENERAL_TIMER, 1000);
	tick_timer_s.tick_ms = 0;
	nrfx_timer_compare(&GENERAL_TIMER, NRF_TIMER_CC_CHANNEL2, tick_timer_s.period, true);
	nrfx_timer_enable(&GENERAL_TIMER);

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
	GENERAL_TIMER.p_reg->CC[1] = 0;
	GENERAL_TIMER.p_reg->TASKS_CAPTURE[NRF_TIMER_CC_CHANNEL1] = 1;
	nrfx_timer_compare(&GENERAL_TIMER, NRF_TIMER_CC_CHANNEL0, slot_timer_s.period + GENERAL_TIMER.p_reg->CC[1], true);
#endif
}

void PORT_SleepMs(time_ms_t time_ms) {
	nrf_delay_ms(time_ms);
}

time_ms_t PORT_TickMs() { return tick_timer_s.tick_ms; }

// get high resosolution clock tick
unsigned int PORT_TickHr() { return DWT->CYCCNT; }

unsigned int PORT_TickHrToUs(unsigned int delta) {		// TODO implement this
	return 0;
}

// return current slot timer tick counter
uint32_t PORT_SlotTimerTickUs() {
#if USE_SLOT_TIMER
	GENERAL_TIMER.p_reg->CC[1] = 0;
	GENERAL_TIMER.p_reg->TASKS_CAPTURE[NRF_TIMER_CC_CHANNEL1] = 1;
	return slot_timer_s.period - (GENERAL_TIMER.p_reg->CC[0] - GENERAL_TIMER.p_reg->CC[1]);
#endif
	return 0;
}

// extend slot timer period for one iteration by delta_us
void PORT_SlotTimerSetUsOffset(int32 delta_us) {
#if USE_SLOT_TIMER
	// get actual timer count to CC[1]
	GENERAL_TIMER.p_reg->TASKS_CAPTURE[NRF_TIMER_CC_CHANNEL1] = 1;
	// changing us to ticks
	delta_us = (delta_us < 0)
			? (int32_t)(-1*nrfx_timer_us_to_ticks(&GENERAL_TIMER, (uint32_t)abs(delta_us)))
			: nrfx_timer_us_to_ticks(&GENERAL_TIMER, delta_us);
	// when actual compare value + delta is more than actual count + 25 (as reserve)
	if(GENERAL_TIMER.p_reg->CC[NRF_TIMER_CC_CHANNEL0] + delta_us > GENERAL_TIMER.p_reg->CC[NRF_TIMER_CC_CHANNEL1] + 50) {
		slot_timer_s.buffer = slot_timer_s.period;
		slot_timer_s.period += delta_us;
		GENERAL_TIMER.p_reg->CC[NRF_TIMER_CC_CHANNEL0] += delta_us;
	}
#endif
}

// set slot timer period
void PORT_SetSlotTimerPeriodUs(uint32 us) {
#if USE_SLOT_TIMER
	// getting actual timer count
	GENERAL_TIMER.p_reg->CC[1] = 0;
	GENERAL_TIMER.p_reg->TASKS_CAPTURE[NRF_TIMER_CC_CHANNEL1] = 1;
	// format us to timer ticks, based on actual frequency
	slot_timer_s.period = nrfx_timer_us_to_ticks(&GENERAL_TIMER, us);
	// adding new period to actual count (as clearing the timer after period change)
	GENERAL_TIMER.p_reg->CC[NRF_TIMER_CC_CHANNEL0] = GENERAL_TIMER.p_reg->CC[1] + slot_timer_s.period;
#endif
}
