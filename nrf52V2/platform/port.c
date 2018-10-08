#include "port.h"
#include "nrfx_gpiote.h"
#include "nrfx_wdt.h"
#include "nrf_sdm.h"
#include "settings.h"
#include "FU.h"

void PORT_TimeInit();
void PORT_GpioInit(bool);
void PORT_SpiInit();
void PORT_AdcInit();
void PORT_CrcInit();
void PORT_ExtiInit(bool);
void PORT_UsbUartInit();

void PORT_Init() {
	APP_ERROR_CHECK(sd_softdevice_vector_table_base_set((uint32_t)(FU_GetCurrentFlashBase())));
	IASSERT(NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY == NRFX_GPIOTE_CONFIG_IRQ_PRIORITY);
	PORT_AdcInit();
	rtls_role role_m = PORT_GetHwRole();
	PORT_BleBeaconInit();
	PORT_TimeInit();
	PORT_UsbUartInit();
	PORT_CrcInit();
	PORT_SpiInit();
	PORT_GpioInit(role_m == RTLS_TAG);
	PORT_ImuInit(role_m == RTLS_TAG);
	PORT_ExtiInit(role_m == RTLS_TAG);
#if !DBG
	PORT_WatchdogInit();
#endif
}

void PORT_GpioInit(bool is_tag) {
	nrf_gpio_cfg_output(LED_ERR);
	nrf_gpio_cfg_output(LED_STAT);
	PORT_LedOff(LED_STAT);
	PORT_LedOff(LED_ERR);
	nrf_gpio_cfg_input(DW_RST_PIN, NRF_GPIO_PIN_NOPULL);
	if(is_tag) {																								// configuring the buzzer's pin
		nrf_gpio_cfg(BUZZER_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_D0H1, NRF_GPIO_PIN_NOSENSE);
		nrf_gpio_pin_clear(BUZZER_PIN);
	}
}

static nrfx_wdt_channel_id m_wdt_channel_id;
void wdt_event_handler(void){ }

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info) {
    IASSERT(0);
}

void PORT_WatchdogInit() {
	nrfx_wdt_config_t wdt_config = NRFX_WDT_DEAFULT_CONFIG;
	APP_ERROR_CHECK(nrfx_wdt_init(&wdt_config, wdt_event_handler));
	APP_ERROR_CHECK(nrfx_wdt_channel_alloc(&m_wdt_channel_id));
	nrfx_wdt_enable();
}

void PORT_WatchdogRefresh() {
#if !DBG
	nrfx_wdt_channel_feed(m_wdt_channel_id);
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

void PORT_BuzzOn() {
	nrf_gpio_pin_set(BUZZER_PIN);
}

void PORT_BuzzOff() {
	nrf_gpio_pin_clear(BUZZER_PIN);
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

void DW_EXTI_IRQ_Handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
	do
	{
		nrf_gpio_pin_set(DW_SPI_SS_PIN);
		dwt_isr();
	} while(nrf_gpio_pin_read(DW_EXTI_IRQn) == 1);
}

#if IMU_EXTI_IRQ1
void ImuIrqHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  PORT_ImuIrqHandler();
}
#endif

void PORT_ExtiInit(bool is_tag) {
    APP_ERROR_CHECK(nrfx_gpiote_init());
    nrfx_gpiote_in_config_t dw_int_config = {
    		.is_watcher = false,
    		.hi_accuracy = false,
    		.pull = NRF_GPIO_PIN_NOPULL,
    		.sense = NRF_GPIOTE_POLARITY_LOTOHI,
    };
    APP_ERROR_CHECK(nrfx_gpiote_in_init(DW_EXTI_IRQn, &dw_int_config, DW_EXTI_IRQ_Handler));
    nrfx_gpiote_in_event_enable(DW_EXTI_IRQn, true);

#if IMU_EXTI_IRQ1
    if (!is_tag) {
  		return;
  	}
	nrfx_gpiote_in_config_t imu_int_config = {
			.is_watcher = false,
			.hi_accuracy = false,
			.pull = NRF_GPIO_PIN_PULLDOWN,
			.sense = NRF_GPIOTE_POLARITY_LOTOHI,
	};
	APP_ERROR_CHECK(nrfx_gpiote_in_init(IMU_EXTI_IRQ1, &imu_int_config, ImuIrqHandler));
	nrfx_gpiote_in_event_enable(IMU_EXTI_IRQ1, true);
#endif
}
