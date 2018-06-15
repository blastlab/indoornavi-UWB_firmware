/*
 * port_mutex.c
 *
 *  Created on: 22.03.2018
 *      Author: KarolTrzcinski
 */

#include "port.h"
#include "nrf_drv_gpiote.h"

// return 0 when IRQ is inactive, 1 otherwise		TODO: implement EXTI_GetITEnStatus - checking if DW_IRQ_PIN interrupt is handled or disabled
uint8_t EXTI_GetITEnStatus(uint32_t IRQn) {
	uint8_t irq_stat = 0;
    ASSERT(m_cb.pin_assignments[IRQn] >= 0);				// checking if pin is in use by gpiote
    if (m_cb.pin_assignments[IRQn] >= GPIOTE_CH_NUM) {		// checking if PORT interrupt is configured
    	irq_stat = (nrf_gpio_pin_sense_get(IRQn) == NRF_GPIO_PIN_NOSENSE) ? 0 : 1;
    }
    else if ((m_cb.pin_assignments[IRQn] >= 0 && m_cb.pin_assignments[IRQn] <	// or if high accuracy interrupt is configured
            GPIOTE_CH_NUM)) {
        int32_t channel = (int32_t)m_cb.pin_assignments[IRQn];					// getting pin's channel
        irq_stat = ((NRF_GPIOTE->CONFIG[channel] & GPIOTE_CONFIG_MODE_Event) == GPIOTE_CONFIG_MODE_Event) ? 1 : 0;		// checking event configuration
    }
    return irq_stat;
}

// get deca spi mutex
decaIrqStatus_t decamutexon(void) {
	  decaIrqStatus_t s = EXTI_GetITEnStatus(DW_EXTI_IRQn);
	  if (s) {
		  nrf_drv_gpiote_in_event_disable(DW_EXTI_IRQn); // NVIC_DisableIRQ; disable the external interrupt line
	  }
	  return s; // return state before disable, value is used to re-enable in
	            // decamutexoff call
}

// release deca spi mutex
void decamutexoff(decaIrqStatus_t s) {
	if (s) { // need to check the port state as we can't use level sensitive
		   	 // interrupt on the STM ARM
		nrf_drv_gpiote_in_event_enable(DW_EXTI_IRQn, true); // NVIC_EnableIRQ()
	}
}
