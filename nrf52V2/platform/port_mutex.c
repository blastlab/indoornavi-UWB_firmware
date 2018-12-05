/*
 * port_mutex.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */

#include "port.h"
#include "nrf52.h"
#include "nrf_nvic.h"

extern volatile bool m_ble_radio_active;

static inline bool is_irq_enabled(IRQn_Type IRQn) {
	return NVIC->ISER[(((uint32_t)(int32_t)IRQn) >> 5UL)] &
	                 (uint32_t)(1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL));
}

decaIrqStatus_t decamutexon(void) {
	decaIrqStatus_t s = is_irq_enabled(GPIOTE_IRQn);
	if (s) {
		NVIC_DisableIRQ(GPIOTE_IRQn);	// DW interrupts
#if USE_SLOT_TIMER
		NVIC_DisableIRQ(TIMER1_IRQn);	// Slot timer
#endif
	}
	return s;
}

// release deca spi mutex
void decamutexoff(decaIrqStatus_t s) {
	if (s) {
		NVIC_EnableIRQ(GPIOTE_IRQn);	// DW interrupts
#if USE_SLOT_TIMER
		NVIC_EnableIRQ(TIMER1_IRQn);	// Slot timer
#endif
	}
}

decaIrqStatus_t PORT_EnterCritical() {
	uint8_t s = 0;
BLE_CODE(
	volatile bool s_swi2 = is_irq_enabled(SWI2_EGU2_IRQn);
	while(m_ble_radio_active);
)
	sd_nvic_critical_region_enter(&s);
	BLE_CODE(
	if(s_swi2) {
		NVIC_EnableIRQ(SWI2_EGU2_IRQn);
	}
)
	return s;
}

void PORT_ExitCritical(decaIrqStatus_t s) {
	sd_nvic_critical_region_exit(s);
}
