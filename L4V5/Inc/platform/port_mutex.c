/*
 * port_mutex.c
 *
 *  Created on: 22.03.2018
 *      Author: KarolTrzcinski
 */

#include "platform/port.h"

// return 0 when IRQ is inactive, 1 otherwise
ITStatus EXTI_GetITEnStatus(uint32_t IRQn) {
  ITStatus bitstatus = RESET;
  uint32_t enablestatus = 0;
  // Check the parameters */
  assert_param(IS_GET_EXTI_LINE(EXTI_Line));

  enablestatus = NVIC->ISER[(((uint32_t)(int32_t)IRQn) >> 5UL)] &
                 (uint32_t)(1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL));

  if (enablestatus != (uint32_t)RESET) {
    bitstatus = SET;
  } else {
    bitstatus = RESET;
  }
  return bitstatus;
}

// get deca spi mutex
decaIrqStatus_t decamutexon(void) {
  decaIrqStatus_t s = EXTI_GetITEnStatus(DW_EXTI_IRQn);

  if (s) {
    NVIC_DisableIRQ(DW_EXTI_IRQn);  // disable the external interrupt line
  }
  return s;  // return state before disable, value is used to re-enable in
             // decamutexoff call
}

// release deca spi mutex
void decamutexoff(decaIrqStatus_t s) {
  if (s) {  // need to check the port state as we can't use level sensitive
            // interrupt on the STM ARM
    NVIC_EnableIRQ(DW_EXTI_IRQn);
  }
}

decaIrqStatus_t PORT_EnterCritical() {
  decaIrqStatus_t s = __get_PRIMASK();
  __disable_irq();
  return s;
}

void PORT_ExitCritical(decaIrqStatus_t s) {
  if (!s) {
    __enable_irq();
  }
}
