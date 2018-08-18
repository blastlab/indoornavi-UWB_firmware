/*
 * port_mutex.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */

#include "port.h"
#include "nrf_nvic.h"

// get deca spi mutex
decaIrqStatus_t decamutexon(void) {
	  uint8_t s = 0;
	  sd_nvic_critical_region_enter(&s);
	  return s;
}

// release deca spi mutex
void decamutexoff(decaIrqStatus_t s) {
	sd_nvic_critical_region_exit(s);
}

decaIrqStatus_t PORT_EnterCritical() {
  return decamutexon();
}

void PORT_ExitCritical(decaIrqStatus_t s) {
	decamutexoff(s);
}
