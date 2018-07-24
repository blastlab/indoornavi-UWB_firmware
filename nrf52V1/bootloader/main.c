#include <stdint.h>
#include "nrf52.h"
#include "bootloader.h"

int main(void)
{
	uint32_t reset_reason = NRF_POWER->RESETREAS;
	NRF_POWER->RESETREAS = 0xFFFFFFFF;

	MainInit();
	Bootloader_Init(reset_reason);
	Bootloader_JumpToApi();

    while (1)
    {
    	NVIC_SystemReset();
    }
}

