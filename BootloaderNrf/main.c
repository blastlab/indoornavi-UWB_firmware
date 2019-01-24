#include <stdint.h>
#include "nrf52.h"
#include "bootloader.h"

// implementation
void MainInit(void) {
	nrf_gpio_cfg_output(LED_G1);
	nrf_gpio_cfg_output(LED_R1);
	nrf_gpio_pin_clear(LED_G1);
	nrf_gpio_pin_clear(LED_R1);
}

int main(void)
{
	uint32_t reset_reason = NRF_POWER->RESETREAS;
	// 1 - Reset Pin
	// 2 - Watchdog
	// 4 - soft reset
	// 8 - CPU lock-up

	NRF_POWER->RESETREAS = 0xFFFFFFFF;

	MainInit();
	Start(reset_reason);

    while (1)
    {
    	NVIC_SystemReset();
    }
}

