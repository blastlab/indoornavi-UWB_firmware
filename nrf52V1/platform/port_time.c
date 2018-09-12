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
#include "nrf_drv_timer.h"
#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"

const nrf_drv_timer_t TIMER_SLOT = NRF_DRV_TIMER_INSTANCE(1);
const nrf_drv_rtc_t RTC = NRF_DRV_RTC_INSTANCE(1);

static void rtc_handler(nrf_drv_rtc_int_type_t int_type) { }
static volatile uint32_t slot_timer_buf;

static void timer_slot_event_handler(nrf_timer_event_t event_type, void* p_context) {
	if(event_type == NRF_TIMER_EVENT_COMPARE0) {
		if(slot_timer_buf != 0) {
			TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL0] = slot_timer_buf;
			slot_timer_buf = 0;
		}
		MAC_YourSlotIsr();
	}
}

void PORT_TimeInit() {
	slot_timer_buf = 0;
	nrf_drv_clock_hfclk_request(NULL);
    nrf_drv_clock_init();					// when using SD, module is already initialized
    nrf_drv_clock_lfclk_request(NULL);
    nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
    APP_ERROR_CHECK(nrf_drv_rtc_init(&RTC, &config, rtc_handler));
//    nrf_drv_rtc_tick_enable(&RTC, true);	// tick event not used
    nrf_drv_rtc_enable(&RTC);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;					// TIMER: f = 1 MHz; T = 1 us
    APP_ERROR_CHECK(nrf_drv_timer_init(&TIMER_SLOT, &timer_cfg, timer_slot_event_handler));
    nrf_drv_timer_extended_compare(
         &TIMER_SLOT, NRF_TIMER_CC_CHANNEL0, nrf_drv_timer_ms_to_ticks(&TIMER_SLOT, 100), NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

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
	if(settings.ble.is_enabled) {
		PORT_BleSetAdvData(settings.mac.addr, 0, 0);
		PORT_BleAdvStart();
	}
#if USE_SLOT_TIMER
	nrf_drv_timer_enable(&TIMER_SLOT);
#endif
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

// return current slot timer tick counter
uint32_t PORT_SlotTimerTickUs() {
	TIMER_SLOT.p_reg->CC[1] = 0;
	TIMER_SLOT.p_reg->TASKS_CAPTURE[NRF_TIMER_CC_CHANNEL1] = 1;
	while(!TIMER_SLOT.p_reg->CC[1]);
	return TIMER_SLOT.p_reg->CC[1];
}

// extend slot timer period for one iteration by delta_us
void PORT_SlotTimerSetUsOffset(int32 delta_us) {
	TIMER_SLOT.p_reg->TASKS_CAPTURE[NRF_TIMER_CC_CHANNEL1] = 1;														// get actual timer count to CC[1]
	if(TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL0] + delta_us > TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL1] + 50) {	// auto_reload + delta > actual_count + 50 (as reserve)
		slot_timer_buf = TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL0];
		TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL0] += delta_us;
	}
}

// set slot timer period
void PORT_SetSlotTimerPeriodUs(uint32 us) {
	TIMER_SLOT.p_reg->CC[NRF_TIMER_CC_CHANNEL0] = nrf_drv_timer_us_to_ticks(&TIMER_SLOT, us);
	TIMER_SLOT.p_reg->TASKS_CLEAR = 1;																				// clearing the timer to prevent waiting for overcount when new period is smaller than actual timer count
}
