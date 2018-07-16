#include <stdbool.h>
#include <stdint.h>
#include "nrf52.h"
#include "bootloader.h"

int main(void)
{
	uint32_t last_RCC_flags = 0; // RCC->CSR;
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
//	HAL_Init();
//	SystemClock_Config();
//	MX_GPIO_Init();
//	MX_RTC_Init();
	Bootloader_Init(last_RCC_flags);
	Bootloader_JumpToApi();

    while (1)
    {
    	NVIC_SystemReset();
    }
}

