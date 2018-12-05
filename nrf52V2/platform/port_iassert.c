/*
 * port_iassert.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */

#include <stdio.h>
#include "../logger/logs.h"
#include "port.h"
#include "nrf52.h"

uint8_t PORT_UsbUartTransmit(uint8_t *buf, uint16_t len);

void PORT_iassert_fun(const char *msg, int line) {
	#define SIZE 200
	int counter = 0;
	int tim = 99999;
	char buf[SIZE];

	int n = snprintf(buf, SIZE, "ASSERT: %s line: %d\r\n", msg, line);
	UNUSED(n);

	// disable all interrupts
	// why to FPU_IRQn? It is the last IRQn for this processor
	for(int i = 1; i <= FPU_IRQn; ++i) {
		if(i == WDT_IRQn || i == UARTE0_UART0_IRQn) continue;			// we do not want to turn off watchdog and UART interrupts
		NVIC_DisableIRQ(i);
	}

	PORT_LedOn(LED_ERR);

	#if DBG == 0
	NVIC_SystemReset();
	#endif

	while(1)
	{
		if(counter++ % 25 == 0)
		{
			#if LOG_USB_EN
				PORT_UsbUartTransmit((uint8_t*)buf, n);
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
