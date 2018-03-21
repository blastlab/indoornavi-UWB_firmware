#include "platform/port.h"

void port_watchdog_refresh()
{
	if(LL_WWDG_GetCounter(WWDG) < LL_WWDG_GetWindow(WWDG)) {
		LL_WWDG_SetCounter(WWDG, 127);
	}
}

void port_sleep_ms(unsigned int time_ms)
{
	HAL_Delay(time_ms);
}

unsigned int port_tick_ms()
{
	return HAL_GetTick();
}

// get high resosolution clock tick
unsigned int port_tick_hr()
{
    return 0;
}

// reset dw 1000 device by polling RST pin down for a few ms
void reset_DW1000()
{
	//May be pulled low by external open drain driver to reset the DW1000.
	//Must not be pulled high by external source.
	// from DW1000 datasheet table 1
	GPIO_InitTypeDef GPIO_InitStructure;

	// Enable GPIO used for DW1000 reset

	GPIO_InitStructure.Pin = DW_RST_Pin;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
	HAL_GPIO_Init(DW_RST_GPIO_Port, &GPIO_InitStructure);

	//drive the RSTn pin low
	HAL_GPIO_WritePin(DW_RST_GPIO_Port, DW_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(2);

	//put the pin back to tri-state ... as input
	GPIO_InitStructure.Pin = DW_RST_Pin;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
	HAL_GPIO_Init(DW_RST_GPIO_Port, &GPIO_InitStructure);

	HAL_Delay(2);
}

// return 0 when IRQ is inactive, 1 otherwise
ITStatus EXTI_GetITEnStatus(uint32_t IRQn)
{
  ITStatus bitstatus = RESET;
  uint32_t enablestatus = 0;
  // Check the parameters */
  assert_param(IS_GET_EXTI_LINE(EXTI_Line));

  enablestatus = NVIC->ISER[(((uint32_t)(int32_t)IRQn) >> 5UL)] & (uint32_t)(1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL));;
  if (enablestatus != (uint32_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }
  return bitstatus;
}

// get deca spi mutex
decaIrqStatus_t decamutexon(void)
{
	decaIrqStatus_t s = EXTI_GetITEnStatus(DW_EXTI_IRQn);

	if(s) {
		NVIC_DisableIRQ(DW_EXTI_IRQn); //disable the external interrupt line
	}
	return s ;   // return state before disable, value is used to re-enable in decamutexoff call
}

// release deca spi mutex
void decamutexoff(decaIrqStatus_t s)
{
	if(s) { //need to check the port state as we can't use level sensitive interrupt on the STM ARM
		NVIC_EnableIRQ(DW_EXTI_IRQn);
}
}
