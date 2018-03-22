#include "platform/port.h"

void port_watchdog_init() {
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_WWDG);
  LL_WWDG_SetCounter(WWDG, 127);
  LL_WWDG_Enable(WWDG);
  LL_WWDG_SetPrescaler(WWDG, LL_WWDG_PRESCALER_8);
  LL_WWDG_SetWindow(WWDG, 127);
}

void port_watchdog_refresh() {
  if (LL_WWDG_GetCounter(WWDG) < LL_WWDG_GetWindow(WWDG)) {
    LL_WWDG_SetCounter(WWDG, 127);
  }
}

// turn led on
void port_led_on(int LED_x) {
  switch (LED_x) {
  case LED_G1:
    LL_GPIO_SetOutputPin(LED_G1_GPIO_Port, LED_G1_Pin);
    break;
  case LED_R1:
    LL_GPIO_SetOutputPin(LED_R1_GPIO_Port, LED_R1_Pin);
    break;
  default:
    IASSERT(0);
    break;
  }
}

// turrn led off
void port_led_off(int LED_x) {
  switch (LED_x) {
  case LED_G1:
    LL_GPIO_ResetOutputPin(LED_G1_GPIO_Port, LED_G1_Pin);
    break;
  case LED_R1:
    LL_GPIO_ResetOutputPin(LED_R1_GPIO_Port, LED_R1_Pin);
    break;
  default:
    IASSERT(0);
    break;
  }
}

// reset dw 1000 device by polling RST pin down for a few ms
void reset_DW1000() {
  // May be pulled low by external open drain driver to reset the DW1000.
  // Must not be pulled high by external source.
  // from DW1000 datasheet table 1
  GPIO_InitTypeDef GPIO_InitStructure;

  // Enable GPIO used for DW1000 reset

  GPIO_InitStructure.Pin = DW_RST_Pin;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
  HAL_GPIO_Init(DW_RST_GPIO_Port, &GPIO_InitStructure);

  // drive the RSTn pin low
  HAL_GPIO_WritePin(DW_RST_GPIO_Port, DW_RST_Pin, GPIO_PIN_RESET);
  port_sleep_ms(2);

  // put the pin back to tri-state ... as input
  GPIO_InitStructure.Pin = DW_RST_Pin;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
  HAL_GPIO_Init(DW_RST_GPIO_Port, &GPIO_InitStructure);

  HAL_Delay(2);
}

void port_reboot() {
  // turn off USB, to reconnect after reset
  // it help from usb timeout error from the host side
  USB_StopDevice(USB);
  USB_DevDisconnect(USB);
  port_sleep_ms(10); // to be sure
  NVIC_SystemReset();
}

void port_enter_stop_mode() {
  __disable_irq();
  port_led_off(LED_R1);
  port_led_off(LED_G1);
  HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
}
