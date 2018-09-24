/*
 * port_spi.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */

#include "port.h"
#include "nrfx_spim.h"
#include "nrf_gpio.h"

// ==== SPI ====
#define DW_SPI_INSTANCE  		0
#define GENERAL_SPI_INSTANCE	1

static const nrfx_spim_t spi0 = NRFX_SPIM_INSTANCE(DW_SPI_INSTANCE);
static const nrfx_spim_t spi1 = NRFX_SPIM_INSTANCE(GENERAL_SPI_INSTANCE);

#define NRFX_SPIM_DW_CONFIG                             		 	\
{                                                            	\
    .sck_pin        = DW_SPI_SCK_PIN,                					\
    .mosi_pin       = DW_SPI_MOSI_PIN,                				\
    .miso_pin       = DW_SPI_MISO_PIN,                				\
    .ss_pin         = NRFX_SPIM_PIN_NOT_USED,                	\
    .ss_active_high = false,                                 	\
    .irq_priority   = NRFX_SPIM_DEFAULT_CONFIG_IRQ_PRIORITY, 	\
    .orc            = 0xFF,                                  	\
    .frequency      = NRF_SPIM_FREQ_8M,                      	\
    .mode           = NRF_SPIM_MODE_0,                       	\
    .bit_order      = NRF_SPIM_BIT_ORDER_MSB_FIRST,          	\
}

#define NRFX_SPIM_GEN_CONFIG                             		 	\
{                                                            	\
    .sck_pin        = GEN_SPI_SCK_PIN,                				\
    .mosi_pin       = GEN_SPI_MOSI_PIN,                				\
    .miso_pin       = GEN_SPI_MISO_PIN,                				\
    .ss_pin         = NRFX_SPIM_PIN_NOT_USED,                	\
    .ss_active_high = false,                                 	\
    .irq_priority   = NRFX_SPIM_DEFAULT_CONFIG_IRQ_PRIORITY, 	\
    .orc            = 0xFF,                                  	\
    .frequency      = NRF_SPIM_FREQ_8M,                      	\
    .mode           = NRF_SPIM_MODE_0,                       	\
    .bit_order      = NRF_SPIM_BIT_ORDER_MSB_FIRST,          	\
}

void PORT_SpiSpeedSlow(bool slow) {
	nrfx_spim_uninit(&spi0);
	nrfx_spim_config_t spi0_config = NRFX_SPIM_DW_CONFIG;
	spi0_config.frequency = (slow) ? NRF_SPIM_FREQ_2M : NRF_SPIM_FREQ_8M;
	APP_ERROR_CHECK(nrfx_spim_init(&spi0, &spi0_config, NULL, NULL));
}

void PORT_SpiInit() {
	nrf_gpio_cfg_output(DW_SPI_SS_PIN);
	nrf_gpio_pin_set(DW_SPI_SS_PIN);
	nrfx_spim_config_t spi0_config = NRFX_SPIM_DW_CONFIG;
	APP_ERROR_CHECK(nrfx_spim_init(&spi0, &spi0_config, NULL, NULL));

#if GEN_SPI_SCK_PIN
	nrfx_spim_config_t spi1_config = NRFX_SPIM_GEN_CONFIG;
	APP_ERROR_CHECK(nrfx_spim_init(&spi1, &spi1_config, NULL, NULL));
#endif
}

#pragma GCC optimize("O3")
inline void PORT_SpiTx(uint32_t length, const uint8_t *buf) {
#if GEN_SPI_SCK_PIN
	IASSERT(nrfx_is_in_ram(buf));
	NRF_SPIM1->TXD.PTR = (uint32_t)buf;
	NRF_SPIM1->TXD.MAXCNT = length;
	NRF_SPIM1->RXD.PTR = (uint32_t)NULL;
	NRF_SPIM1->RXD.MAXCNT = 0;
	NRF_SPIM1->EVENTS_END = 0x0UL;
	NRF_SPIM1->TXD.LIST = 0x0UL;
	NRF_SPIM1->RXD.LIST = 0x0UL;
	NRF_SPIM1->TASKS_START = 0x1UL;
	while(NRF_SPIM1->EVENTS_END == 0x0UL);
#endif
}
#pragma GCC optimize("O3")
inline void PORT_SpiRx(uint32_t length, uint8_t *buf) {
#if GEN_SPI_SCK_PIN
	IASSERT(nrfx_is_in_ram(buf));
	NRF_SPIM1->TXD.PTR = (uint32_t)NULL;
	NRF_SPIM1->TXD.MAXCNT = 0;
	NRF_SPIM1->RXD.PTR = (uint32_t)buf;
	NRF_SPIM1->RXD.MAXCNT = length;
	NRF_SPIM1->EVENTS_END = 0x0UL;
	NRF_SPIM1->TXD.LIST = 0x0UL;
	NRF_SPIM1->RXD.LIST = 0x0UL;
	NRF_SPIM1->TASKS_START = 0x1UL;
	while(NRF_SPIM1->EVENTS_END == 0x0UL);
#endif
}

#pragma GCC optimize("O3")
inline static void dw_spiTx(uint32_t length, const uint8_t *buf) {
	IASSERT(nrfx_is_in_ram(buf));
	NRF_SPIM0->TXD.PTR = (uint32_t)buf;
	NRF_SPIM0->TXD.MAXCNT = length;
	NRF_SPIM0->RXD.PTR = (uint32_t)NULL;
	NRF_SPIM0->RXD.MAXCNT = 0;
	NRF_SPIM0->EVENTS_END = 0x0UL;
	NRF_SPIM0->TXD.LIST = 0x0UL;
	NRF_SPIM0->RXD.LIST = 0x0UL;
	NRF_SPIM0->TASKS_START = 0x1UL;
	while(NRF_SPIM0->EVENTS_END == 0x0UL);
}
#pragma GCC optimize("O3")
inline static void dw_spiRx(uint32_t length, uint8_t *buf) {
	IASSERT(nrfx_is_in_ram(buf));
	NRF_SPIM0->TXD.PTR = (uint32_t)NULL;
	NRF_SPIM0->TXD.MAXCNT = 0;
	NRF_SPIM0->RXD.PTR = (uint32_t)buf;
	NRF_SPIM0->RXD.MAXCNT = length;
	NRF_SPIM0->EVENTS_END = 0x0UL;
	NRF_SPIM0->TXD.LIST = 0x0UL;
	NRF_SPIM0->RXD.LIST = 0x0UL;
	NRF_SPIM0->TASKS_START = 0x1UL;
	while(NRF_SPIM0->EVENTS_END == 0x0UL);
}

#pragma GCC optimize("O3")
int readfromspi(uint16_t headerLength, const uint8_t *headerBuffer,
                uint32_t bodyLength, uint8_t *bodyBuffer) {
	  decaIrqStatus_t en = decamutexon();
	  nrf_gpio_pin_clear(DW_SPI_SS_PIN);
	  dw_spiTx(headerLength, headerBuffer);
	  dw_spiRx(bodyLength, bodyBuffer);
	  nrf_gpio_pin_set(DW_SPI_SS_PIN);
	  decamutexoff(en);
	  return 0;
}

#pragma GCC optimize("O3")
int writetospi(uint16_t headerLength, const uint8_t *headerBuffer,
               uint32_t bodyLength, const uint8_t *bodyBuffer) {
	  decaIrqStatus_t en = decamutexon();
	  nrf_gpio_pin_clear(DW_SPI_SS_PIN);
	  dw_spiTx(headerLength, headerBuffer);
	  dw_spiTx(bodyLength, bodyBuffer);
	  nrf_gpio_pin_set(DW_SPI_SS_PIN);
	  decamutexoff(en);
	  return 0;
}

void PORT_WakeupTransceiver(void)
{
	nrf_gpio_pin_clear(DW_SPI_SS_PIN);
	PORT_SleepMs(2);
	nrf_gpio_pin_set(DW_SPI_SS_PIN);
	PORT_SleepMs(7); 					// wait 7ms for DW1000 XTAL to stabilise
}
