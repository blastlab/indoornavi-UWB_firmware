/*
 * port_iassert.c
 *
 *  Created on: 16.05.2018
 *      Author: KarolTrzcinski
 */


#include "logs.h"
#include "platform/port.h"

void PORT_iassert_fun(const char *msg, int line) {
	#define SIZE 200
	int counter = 0;
	int tim = 99999;
	char buf[SIZE];
	int n = snprintf(buf, SIZE, "ASSERT: %s line: %d\r\n", msg, line);
	UNUSED(n);

	// disable all interrupts
	// why to CRS_IRQn? It is the last IRQn for this processor
	for(int i = 1; i <= CRS_IRQn; ++i) {
		NVIC_DisableIRQ(i);
	}

	NVIC_EnableIRQ(SysTick_IRQn);
	PORT_LedOn(LED_ERR);

	#if NDBG
	NVIC_SystemReset();
	#endif

	while(1)
	{
		if(counter++ % 100 == 0)
		{
			#if LOG_USB_EN
				CDC_Transmit_FS((uint8_t*)buf, n);
			#endif
			#if LOG_UART_EN
				HAL_UART_Transmit(_LOG_HUART, (uint8_t*)buf, strlen(buf), 100);
			#endif
			#if LOG_LCD_EN
				lcd_err(buf);
			#endif
			#if LOG_SD_EN
			#endif
		}

		PORT_LedOff(LED_ERR);
		for(volatile int i = tim; i > 0; --i) {
			PORT_WatchdogRefresh();
		}

		PORT_LedOn(LED_ERR);
		for(volatile int i = tim; i > 0; --i) {
			PORT_WatchdogRefresh();
		}
	}
}