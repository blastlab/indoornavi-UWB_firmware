#include "port.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_wdt.h"

void PORT_TimeInit();
void PORT_GpioInit();
void PORT_SpiInit();
void PORT_BatteryInit();
void PORT_CrcInit();
void PORT_ExtiInit();
void PORT_UsbUartInit();

void PORT_Init() {
	PORT_TimeInit();
	PORT_GpioInit();
	PORT_SpiInit();
	PORT_UsbUartInit();
	PORT_BatteryInit();
	PORT_CrcInit();
	PORT_ExtiInit();
#if !DBG
	PORT_WatchdogInit();
#endif
}

void PORT_GpioInit() {
	nrf_gpio_cfg_output(LED_ERR);
	nrf_gpio_cfg_output(LED_STAT);
	nrf_gpio_cfg_output(LED_BLE);
	nrf_gpio_pin_set(LED_ERR);
	nrf_gpio_pin_set(LED_STAT);
	nrf_gpio_pin_set(LED_BLE);
}

static nrf_drv_wdt_channel_id m_wdt_channel_id;

void wdt_event_handler(void){ }

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info) {
    while(1);
}

void PORT_WatchdogInit() {
	nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
	APP_ERROR_CHECK(nrf_drv_wdt_init(&config, wdt_event_handler));
	APP_ERROR_CHECK(nrf_drv_wdt_channel_alloc(&m_wdt_channel_id));
	nrf_drv_wdt_enable();
}

void PORT_WatchdogRefresh() {
	nrf_drv_wdt_channel_feed(m_wdt_channel_id);
}

// turn led on
void PORT_LedOn(int LED_x) {
	switch(LED_x){
		case LED_G1:
			nrf_gpio_pin_clear(LED_G1);
			break;
		case LED_R1:
			nrf_gpio_pin_clear(LED_R1);
			break;
		case LED_B1:
			nrf_gpio_pin_clear(LED_B1);
			break;
		default:
			IASSERT(0);
			break;
	}
}

// turrn led off
void PORT_LedOff(int LED_x) {
	switch(LED_x){
		case LED_G1:
			nrf_gpio_pin_set(LED_G1);
			break;
		case LED_R1:
			nrf_gpio_pin_set(LED_R1);
			break;
		case LED_B1:
			nrf_gpio_pin_set(LED_B1);
			break;
		default:
			IASSERT(0);
			break;
	}
}

// reset dw 1000 device by polling RST pin down for a few ms
void PORT_ResetTransceiver() {
	nrf_gpio_cfg_output(DW_RST_PIN);
	nrf_gpio_pin_clear(DW_RST_PIN);
	PORT_SleepMs(2);
	nrf_gpio_cfg_default(DW_RST_PIN);
	nrf_gpio_cfg_input(DW_RST_PIN, NRF_GPIO_PIN_NOPULL);
	PORT_SleepMs(2);
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
