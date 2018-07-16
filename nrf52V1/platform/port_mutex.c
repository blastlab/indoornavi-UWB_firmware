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
	nrf_gpio_pin_sense_t irq_stat = nrf_gpio_pin_sense_get(DW_EXTI_IRQn);
    return (irq_stat == NRF_GPIO_PIN_NOSENSE) ? 0 : 1;
}

// get deca spi mutex
decaIrqStatus_t decamutexon(void) {
	  decaIrqStatus_t s = EXTI_GetITEnStatus(DW_EXTI_IRQn);
	  if (s) {
		  nrf_gpio_cfg_sense_set(DW_EXTI_IRQn, NRF_GPIO_PIN_NOSENSE);
	  }
	  return s; // return state before disable, value is used to re-enable in
	            // decamutexoff call
}

// release deca spi mutex
void decamutexoff(decaIrqStatus_t s) {
	if (s) { // need to check the port state as we can't use level sensitive
		   	 // interrupt on the STM ARM
		nrf_gpio_cfg_sense_set(DW_EXTI_IRQn, NRF_GPIO_PIN_SENSE_HIGH);
	}
}
