/*
 * port_mutex.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */

#include "port.h"
#include "nrf_drv_gpiote.h"

// return 0 when IRQ is inactive, 1 otherwise
uint8_t EXTI_GetITEnStatus(uint32_t IRQn) {
	uint8_t irq_stat = 0;
    int32_t channel = (int32_t)m_cb.pin_assignments[IRQn];			// channel_port_get
    nrf_drv_gpiote_evt_handler_t handler = m_cb.handlers[channel]; 	// channel_handler_get(channel_port_get(PIN));
	if (handler)													// return status of the interrupt only if event handler was provided
	{
		irq_stat =  nrf_gpiote_int_is_enabled(1 << channel);
	}
    return irq_stat;
}

// get deca spi mutex
decaIrqStatus_t decamutexon(void) {
	  decaIrqStatus_t s = EXTI_GetITEnStatus(DW_EXTI_IRQn);
	  if (s) {
		  nrf_drv_gpiote_in_event_disable(DW_EXTI_IRQn);
	  }
	  return s; // return state before disable, value is used to re-enable in
	            // decamutexoff call
}

// release deca spi mutex
void decamutexoff(decaIrqStatus_t s) {
	if (s) { // need to check the port state as we can't use level sensitive
		   	 // interrupt on the STM ARM
		nrf_drv_gpiote_in_event_enable(DW_EXTI_IRQn, true);
	}
}
