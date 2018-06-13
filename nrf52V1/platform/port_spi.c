/*
 * port_spi.c
 *
 *  Created on: 21.03.2018
 *      Author: KarolTrzcinski
 */

#include "port.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"

// ==== SPI ====

#define SPI_INSTANCE  0 /**< SPI instance index. */
static const nrf_drv_spi_t spi1 = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);
static volatile bool spi_xfer_done;



void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
}

void PORT_SpiSpeedSlow(bool slow) {

}

void PORT_SpiInit() {
	nrf_gpio_cfg_output(DW_SPI_SS_PIN);
	nrf_gpio_pin_set(DW_SPI_SS_PIN);
	nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
	spi_config.ss_pin   = NRF_DRV_SPI_PIN_NOT_USED;
	spi_config.miso_pin = DW_SPI_MISO_PIN;
	spi_config.mosi_pin = DW_SPI_MOSI_PIN;
	spi_config.sck_pin  = DW_SPI_SCK_PIN;
	APP_ERROR_CHECK(nrf_drv_spi_init(&spi1, &spi_config, spi_event_handler, NULL));
}

#pragma GCC optimize("O3")
static inline void spi_tx(uint32_t length, const uint8_t *buf) {
    spi_xfer_done = false;
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi1, buf, length, NULL, 0));
    while (!spi_xfer_done);
}

#pragma GCC optimize("O3")
static inline void spi_rx(uint32_t length, uint8_t *buf) {
    spi_xfer_done = false;
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi1,NULL, 0, buf, length));
    while (!spi_xfer_done);
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
