#include "port.h"
#include "nrf_drv_gpiote.h"

void PORT_BatteryInit();
void PORT_SpiInit();
void PORT_CrcInit();
void PORT_TimeInit();
void PORT_ExtiInit();

void PORT_Init() {
  PORT_SpiInit();
  PORT_BatteryInit();
  PORT_CrcInit();
  PORT_TimeInit();
  PORT_ExtiInit();
#if !DBG
  PORT_WatchdogInit();
#endif
}

void PORT_WatchdogInit() {

}

void PORT_WatchdogRefresh() {

}

// turn led on
void PORT_LedOn(int LED_x) {

}

// turrn led off
void PORT_LedOff(int LED_x) {

}

// reset dw 1000 device by polling RST pin down for a few ms
void PORT_ResetTransceiver() {

}

void PORT_EnterStopMode() {

}

void DW_EXTI_IRQ_Handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
	do
	{
		nrf_gpio_pin_set(DW_SPI_SS_PIN);
		dwt_isr();
	} while(nrf_gpio_pin_read(DW_EXTI_IRQn) == 1);
}

void PORT_ExtiInit() {
    ret_code_t err_code;
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_config_t in_config = {
		.is_watcher = false,
		.hi_accuracy = true,
		.pull = NRF_GPIO_PIN_PULLDOWN,
		.sense = NRF_GPIOTE_POLARITY_LOTOHI,
    };
    in_config.pull = NRF_GPIO_PIN_PULLUP;
    err_code = nrf_drv_gpiote_in_init(DW_EXTI_IRQn, &in_config, DW_EXTI_IRQ_Handler);
    APP_ERROR_CHECK(err_code);
    NVIC_SetPriority(GPIOTE_IRQn, 5);
    nrf_drv_gpiote_in_event_enable(DW_EXTI_IRQn, true);
}
