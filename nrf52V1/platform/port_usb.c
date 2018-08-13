/*
 * port_usb.c
 *
 *  Created on: 22 cze 2018
 *      Author: DawidPeplinski
 */

#include "port.h"
#include "txt_parser.h"
#include "string.h"
#include "nrf_uart.h"
#include "app_uart.h"

#define MAX_TEST_DATA_BYTES     (15U)                /**< max number of test bytes to be used for tx and rx. */
#define UART_TX_BUF_SIZE 		256	                 /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 		256 	             /**< UART RX buffer size. */

void uart_error_handle(app_uart_evt_t * p_event) {
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
//        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
//        APP_ERROR_HANDLER(p_event->data.error_code);
    }
    else if (p_event->evt_type == APP_UART_DATA_READY)
    {
    	uint8_t byte;
    	app_uart_get(&byte);
    	TXT_Input((char *)&byte, 1);
    }
}

void PORT_UsbUartInit(void) {
	uint32_t err_code;
	const app_uart_comm_params_t comm_params = {
			.rx_pin_no = USB_UART_RX_PIN,
			.tx_pin_no = USB_UART_TX_PIN,
			.rts_pin_no = 0,
			.cts_pin_no = 0,
			.flow_control =  APP_UART_FLOW_CONTROL_DISABLED,
			.use_parity = false,
			.baud_rate = NRF_UART_BAUDRATE_115200
	  };
	APP_UART_FIFO_INIT(&comm_params,
						 UART_RX_BUF_SIZE,
						 UART_TX_BUF_SIZE,
						 uart_error_handle,
						 APP_IRQ_PRIORITY_LOWEST,
						 err_code);
	APP_ERROR_CHECK(err_code);
}

uint8_t PORT_UsbUartTransmit(uint8_t *buf, uint16_t len) {
	for(uint16_t i = 0; i < len; i++) {
		app_uart_put(*(buf + i)) ;
	}
	return PORT_Success;
}
