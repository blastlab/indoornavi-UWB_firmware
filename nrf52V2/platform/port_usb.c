/*
 * port_usb.c
 *
 *  Created on: 22 cze 2018
 *      Author: DawidPeplinski
 */

#include "port.h"
#include "txt_parser.h"
#include "nrfx_uarte.h"
#include "nrf52.h"

#define NRFX_UARTE_CONFIG                                                   				\
{                                                                                   \
    .pseltxd            = USB_UART_TX_PIN,                              						\
    .pselrxd            = USB_UART_RX_PIN,                              						\
    .pselcts            = NRF_UARTE_PSEL_DISCONNECTED,                              \
    .pselrts            = NRF_UARTE_PSEL_DISCONNECTED,                              \
    .p_context          = NULL,                                                     \
    .hwfc               = (nrf_uarte_hwfc_t)NRFX_UARTE_DEFAULT_CONFIG_HWFC,         \
    .parity             = (nrf_uarte_parity_t)NRFX_UARTE_DEFAULT_CONFIG_PARITY,     \
    .baudrate           = (nrf_uarte_baudrate_t)NRFX_UARTE_DEFAULT_CONFIG_BAUDRATE, \
    .interrupt_priority = NRFX_UARTE_DEFAULT_CONFIG_IRQ_PRIORITY,                   \
}

static const nrfx_uarte_t uarte0 = NRFX_UARTE_INSTANCE(0);
void uarte0_handler(nrfx_uarte_event_t const * p_event, void *p_context) { }

void PORT_UsbUartInit(void) {
	nrfx_uarte_config_t uarte0_config = NRFX_UARTE_CONFIG;
	nrfx_uarte_init(&uarte0, &uarte0_config, uarte0_handler);
}

uint8_t PORT_UsbUartTransmit(uint8_t *buf, uint16_t len) {
	IASSERT(nrfx_is_in_ram(buf));
	NRF_UARTE0->TXD.PTR = (uint32_t)buf;
	NRF_UARTE0->TXD.MAXCNT = len;
	while(nrfx_uarte_tx_in_progress(&uarte0));
	NRF_UARTE0->TASKS_STARTTX = 0x1UL;
	while(NRF_UARTE0->EVENTS_TXSTARTED == 0x0UL);
	return PORT_Success;
}
