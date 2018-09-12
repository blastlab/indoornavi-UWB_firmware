#include "port.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_wdt.h"
#include "settings.h"

void PORT_TimeInit();
void PORT_GpioInit();
void PORT_SpiInit();
void PORT_BatteryInit();
void PORT_CrcInit();
void PORT_ExtiInit();
void PORT_UsbUartInit();

void PORT_Init() {
	PORT_BleBeaconInit();
	PORT_BatteryInit();
	PORT_TimeInit();
	PORT_UsbUartInit();
	PORT_CrcInit();
	PORT_SpiInit();
	PORT_GpioInit();
	PORT_ImuInit();
	PORT_ExtiInit();
#if !DBG
	PORT_WatchdogInit();
#endif
}

void PORT_GpioInit() {
	nrf_gpio_cfg_output(LED_ERR);
	nrf_gpio_cfg_output(LED_STAT);
	PORT_LedOff(LED_STAT);
	PORT_LedOff(LED_ERR);
	nrf_gpio_cfg_input(DW_RST_PIN, NRF_GPIO_PIN_NOPULL);
}

static nrf_drv_wdt_channel_id m_wdt_channel_id;
void wdt_event_handler(void){ }

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info) {
    IASSERT(0);
}

void PORT_WatchdogInit() {
	nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
	APP_ERROR_CHECK(nrf_drv_wdt_init(&config, wdt_event_handler));
	APP_ERROR_CHECK(nrf_drv_wdt_channel_alloc(&m_wdt_channel_id));
	nrf_drv_wdt_enable();
}

void PORT_WatchdogRefresh() {
#if !DBG
	nrf_drv_wdt_channel_feed(m_wdt_channel_id);
#endif
}

// turn led on
void PORT_LedOn(int LED_x) {
	switch(LED_x){
		case LED_G1:
		case LED_R1:
#if USE_DECA_DEVKIT
			nrf_gpio_pin_clear(LED_x);
#else
			nrf_gpio_pin_set(LED_x);
#endif
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
		case LED_R1:
#if USE_DECA_DEVKIT
			nrf_gpio_pin_set(LED_x);
#else
			nrf_gpio_pin_clear(LED_x);
#endif
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
	PORT_SleepMs(3);
	nrf_gpio_cfg_default(DW_RST_PIN);
	nrf_gpio_cfg_input(DW_RST_PIN, NRF_GPIO_PIN_NOPULL);
}

void PORT_EnterStopMode() {

}

void PORT_Reboot() {
	PORT_SleepMs(25);
	NVIC_SystemReset();
}

void DW_EXTI_IRQ_Handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
	do
	{
		nrf_gpio_pin_set(DW_SPI_SS_PIN);
		dwt_isr();
	} while(nrf_gpio_pin_read(DW_EXTI_IRQn) == 1);
}

#if IMU_EXTI_IRQ1
void ImuIrqHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  PORT_ImuIrqHandler();
}
#endif

void PORT_ExtiInit() {
    APP_ERROR_CHECK(nrf_drv_gpiote_init());
    nrf_drv_gpiote_in_config_t dw_int_config = {
		.is_watcher = false,
		.hi_accuracy = false,
		.pull = NRF_GPIO_PIN_NOPULL,
		.sense = NRF_GPIOTE_POLARITY_LOTOHI,
    };
    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(DW_EXTI_IRQn, &dw_int_config, DW_EXTI_IRQ_Handler));
    nrf_drv_gpiote_in_event_enable(DW_EXTI_IRQn, true);

#if IMU_EXTI_IRQ1
    if (PORT_GetHwRole() != RTLS_TAG) {
  		return;
  	}
	nrf_drv_gpiote_in_config_t imu_int_config = {
		.is_watcher = false,
		.hi_accuracy = false,
		.pull = NRF_GPIO_PIN_PULLDOWN,
		.sense = NRF_GPIOTE_POLARITY_LOTOHI,
	};
	APP_ERROR_CHECK(nrf_drv_gpiote_in_init(IMU_EXTI_IRQ1, &imu_int_config, ImuIrqHandler));
	nrf_drv_gpiote_in_event_enable(IMU_EXTI_IRQ1, true);
#endif
}
