/*
 * port_mutex.c
 *
 *  Created on: 22.03.2018
 *      Author: KarolTrzcinski
 */

#include "port.h"

// return 0 when IRQ is inactive, 1 otherwise		TODO: implement EXTI_GetITEnStatus - checking if DW_IRQ_PIN interrupt is handled or disabled
//ITStatus EXTI_GetITEnStatus(uint32_t IRQn) {
//  ITStatus bitstatus = RESET;
//  uint32_t enablestatus = 0;
//  // Check the parameters */
//  assert_param(IS_GET_EXTI_LINE(EXTI_Line));
//
//  enablestatus = NVIC->ISER[(((uint32_t)(int32_t)IRQn) >> 5UL)] &
//                 (uint32_t)(1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL));
//  ;
//  if (enablestatus != (uint32_t)RESET) {
//    bitstatus = SET;
//  } else {
//    bitstatus = RESET;
//  }
//  return bitstatus;
//}

// get deca spi mutex
decaIrqStatus_t decamutexon(void) {
  return 0;
}

// release deca spi mutex
void decamutexoff(decaIrqStatus_t s) {

}
