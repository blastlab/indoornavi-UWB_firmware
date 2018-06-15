/*
 * port_spi.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */

#include "port.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"

// ==== SPI ====

#define SPI_INSTANCE  0 /**< SPI instance index. */
static const nrf_drv_spi_t spi0 = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);

#define NRF_DRV_SPI_DW_CONFIG                            	\
{                                                        	\
    .sck_pin      = DW_SPI_SCK_PIN,                			\
    .mosi_pin     = DW_SPI_MOSI_PIN,                		\
    .miso_pin     = DW_SPI_MISO_PIN,                		\
    .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,				\
    .irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY,		\
    .orc          = 0xFF,									\
    .frequency    = NRF_DRV_SPI_FREQ_8M,					\
    .mode         = NRF_DRV_SPI_MODE_0,						\
    .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,		\
}

void PORT_SpiSpeedSlow(bool slow) {
	nrf_drv_spi_uninit(&spi0);
	nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DW_CONFIG;
	spi_config.frequency = (slow) ? NRF_SPI_FREQ_2M : NRF_SPI_FREQ_8M;
	APP_ERROR_CHECK(nrf_drv_spi_init(&spi0, &spi_config, NULL, NULL));
}

void PORT_SpiInit() {
	nrf_gpio_cfg_output(DW_SPI_SS_PIN);
	nrf_gpio_pin_set(DW_SPI_SS_PIN);
	nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DW_CONFIG;
	APP_ERROR_CHECK(nrf_drv_spi_init(&spi0, &spi_config, NULL, NULL));
}

#pragma GCC optimize("O3")
static inline void spi_tx(uint32_t length, const uint8_t *buf) {
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi0, buf, length, NULL, 0));
}

#pragma GCC optimize("O3")
static inline void spi_rx(uint32_t length, uint8_t *buf) {
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi0, NULL, 0, buf, length));
}

#pragma GCC optimize("O3")
int readfromspi(uint16_t headerLength, const uint8_t *headerBuffer,
                uint32_t bodyLength, uint8_t *bodyBuffer) {
	  decaIrqStatus_t en = decamutexon();
	  nrf_gpio_pin_clear(DW_SPI_SS_PIN);
	  spi_tx(headerLength, headerBuffer);
	  spi_rx(bodyLength, bodyBuffer);
	  nrf_gpio_pin_set(DW_SPI_SS_PIN);
	  decamutexoff(en);
	  return 0;
}

#pragma GCC optimize("O3")
int writetospi(uint16_t headerLength, const uint8_t *headerBuffer,
               uint32_t bodyLength, const uint8_t *bodyBuffer) {

	  decaIrqStatus_t en = decamutexon();
	  nrf_gpio_pin_clear(DW_SPI_SS_PIN);
	  spi_tx(headerLength, headerBuffer);
	  spi_tx(bodyLength, bodyBuffer);
	  nrf_gpio_pin_set(DW_SPI_SS_PIN);
	  decamutexoff(en);
	  return 0;
}

void PORT_WakeupTransceiver(void)
{
	nrf_gpio_pin_clear(DW_SPI_SS_PIN);
	PORT_SleepMs(1);
	nrf_gpio_pin_set(DW_SPI_SS_PIN);
	PORT_SleepMs(7); //wait 7ms for DW1000 XTAL to stabilise
}
