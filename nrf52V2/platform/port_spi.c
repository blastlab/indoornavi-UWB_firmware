/*
 * port_spi.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */

#include "port.h"
#include "nrfx_spim.h"
#include "nrfx_gpiote.h"
#include "nrf_delay.h"

void PORT_SpiSpeedSlow(bool slow) {
	NRF_SPIM0->ENABLE = (SPIM_ENABLE_ENABLE_Disabled << SPIM_ENABLE_ENABLE_Pos);
	NRF_SPIM0->FREQUENCY = (slow) ? NRF_SPIM_FREQ_2M : NRF_SPIM_FREQ_8M;
	NRF_SPIM0->ENABLE = (SPIM_ENABLE_ENABLE_Enabled << SPIM_ENABLE_ENABLE_Pos);
}

#if LOG_SPI_EN
// from platfrom/logs.c
void SpiSlaveRequest();
static void EthSpiSlaveirq(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
	if(pin == ETH_SPI_SLAVE_IRQ) {
		SpiSlaveRequest();
	}
}
#endif

static void dwSpiInit(void) {
	// SCK, SPI mode 0
	nrf_gpio_pin_clear(GEN_SPI_SCK_PIN);
  nrf_gpio_cfg(GEN_SPI_SCK_PIN,
               NRF_GPIO_PIN_DIR_OUTPUT,
               NRF_GPIO_PIN_INPUT_DISCONNECT,
               NRF_GPIO_PIN_NOPULL,
               NRF_GPIO_PIN_H0H1,
               NRF_GPIO_PIN_NOSENSE);
  // MOSI
  nrf_gpio_pin_clear(GEN_SPI_MOSI_PIN);
  nrf_gpio_cfg(GEN_SPI_MOSI_PIN,
               NRF_GPIO_PIN_DIR_OUTPUT,
               NRF_GPIO_PIN_INPUT_DISCONNECT,
               NRF_GPIO_PIN_NOPULL,
               NRF_GPIO_PIN_H0H1,
               NRF_GPIO_PIN_NOSENSE);
  // MISO
  nrf_gpio_cfg_input(GEN_SPI_MISO_PIN, (nrf_gpio_pin_pull_t)NRFX_SPIM_MISO_PULL_CFG);
  // CS
	nrf_gpio_pin_set(DW_SPI_SS_PIN);
  nrf_gpio_cfg_output(DW_SPI_SS_PIN);
  NRF_SPIM0->PSEL.SCK = DW_SPI_SCK_PIN;
  NRF_SPIM0->PSEL.MOSI = DW_SPI_MOSI_PIN;
  NRF_SPIM0->PSEL.MISO = DW_SPI_MISO_PIN;
  NRF_SPIM0->FREQUENCY = NRF_SPIM_FREQ_8M;
  NRF_SPIM0->CONFIG = SPIM_CONFIG_ORDER_MsbFirst |
  		(SPIM_CONFIG_CPOL_ActiveHigh << SPIM_CONFIG_CPOL_Pos) |
      (SPIM_CONFIG_CPHA_Leading << SPIM_CONFIG_CPHA_Pos);
  NRF_SPIM0->ORC = 0xFF;
  NRF_SPIM0->ENABLE = (SPIM_ENABLE_ENABLE_Enabled << SPIM_ENABLE_ENABLE_Pos);
}

void genSpiInit(void) {
	// SCK, SPI mode 0
	nrf_gpio_pin_clear(GEN_SPI_SCK_PIN);
  nrf_gpio_cfg(GEN_SPI_SCK_PIN,
               NRF_GPIO_PIN_DIR_OUTPUT,
               NRF_GPIO_PIN_INPUT_DISCONNECT,
               NRF_GPIO_PIN_NOPULL,
               NRF_GPIO_PIN_H0H1,
               NRF_GPIO_PIN_NOSENSE);
  // MOSI
  nrf_gpio_pin_clear(GEN_SPI_MOSI_PIN);
  nrf_gpio_cfg(GEN_SPI_MOSI_PIN,
               NRF_GPIO_PIN_DIR_OUTPUT,
               NRF_GPIO_PIN_INPUT_DISCONNECT,
               NRF_GPIO_PIN_NOPULL,
               NRF_GPIO_PIN_H0H1,
               NRF_GPIO_PIN_NOSENSE);
  // MISO
  nrf_gpio_cfg_input(GEN_SPI_MISO_PIN, (nrf_gpio_pin_pull_t)NRFX_SPIM_MISO_PULL_CFG);
#if ETH_SPI_SS_PIN
  // CS
	nrf_gpio_pin_set(ETH_SPI_SS_PIN);
  nrf_gpio_cfg_output(ETH_SPI_SS_PIN);
#endif
  NRF_SPIM1->PSEL.SCK = GEN_SPI_SCK_PIN;
  NRF_SPIM1->PSEL.MOSI = GEN_SPI_MOSI_PIN;
  NRF_SPIM1->PSEL.MISO = GEN_SPI_MISO_PIN;
  NRF_SPIM1->FREQUENCY = NRF_SPIM_FREQ_4M;
  NRF_SPIM1->CONFIG = SPIM_CONFIG_ORDER_MsbFirst |
  		(SPIM_CONFIG_CPOL_ActiveHigh << SPIM_CONFIG_CPOL_Pos) |
      (SPIM_CONFIG_CPHA_Leading << SPIM_CONFIG_CPHA_Pos);
  NRF_SPIM1->ORC = 0xFF;
  NRF_SPIM1->ENABLE = (SPIM_ENABLE_ENABLE_Enabled << SPIM_ENABLE_ENABLE_Pos);
}
void PORT_SpiInit(bool isSink) {
	dwSpiInit();
#if GEN_SPI_SCK_PIN
	genSpiInit();
#endif
#if LOG_SPI_EN && ETH_SPI_SLAVE_IRQ
	if(isSink) {
    nrfx_gpiote_in_config_t spi_slave_irq_config = {
    		.is_watcher = false,
    		.hi_accuracy = true,
    		.pull = NRF_GPIO_PIN_NOPULL,
    		.sense = NRF_GPIOTE_POLARITY_HITOLO,
    };
    APP_ERROR_CHECK(nrfx_gpiote_in_init(ETH_SPI_SLAVE_IRQ, &spi_slave_irq_config, EthSpiSlaveirq));
		nrfx_gpiote_in_event_enable(ETH_SPI_SLAVE_IRQ, true);
	}
#endif
}

#pragma GCC optimize("O3")
inline void PORT_SpiTx(const uint8_t* buf, uint32_t length, uint32_t cs_pin) {
#if GEN_SPI_SCK_PIN
	nrf_gpio_pin_clear(cs_pin);
	nrf_delay_us(5);
	NRF_SPIM1->TXD.PTR = (uint32_t)buf;
	NRF_SPIM1->TXD.MAXCNT = length;
	NRF_SPIM1->RXD.MAXCNT = 0;
	NRF_SPIM1->EVENTS_END = 0x0UL;
	NRF_SPIM1->TASKS_START = 0x1UL;
	while(NRF_SPIM1->EVENTS_END == 0x0UL);
	nrf_gpio_pin_set(cs_pin);
#endif
}

#pragma GCC optimize("O3")
inline void PORT_SpiRx(uint8_t* buf, int length, int cs_pin) {
#if GEN_SPI_SCK_PIN
	nrf_gpio_pin_clear(cs_pin);
	nrf_delay_us(5);
	NRF_SPIM1->RXD.PTR = (uint32_t)buf;
	NRF_SPIM1->RXD.MAXCNT = length;
	NRF_SPIM1->TXD.MAXCNT = 0;
	NRF_SPIM1->EVENTS_END = 0x0UL;
	NRF_SPIM1->TASKS_START = 0x1UL;
	while(NRF_SPIM1->EVENTS_END == 0x0UL);
	nrf_gpio_pin_set(cs_pin);
#endif
}

#pragma GCC optimize("O3")
inline void PORT_SpiTxRx(uint8_t* tx_buf, int tx_length, uint8_t* rx_buf, int rx_length, int cs_pin) {
#if GEN_SPI_SCK_PIN
	nrf_gpio_pin_clear(cs_pin);
	// tx data
	NRF_SPIM1->TXD.PTR = (uint32_t)tx_buf;
	NRF_SPIM1->TXD.MAXCNT = tx_length;
	NRF_SPIM1->RXD.MAXCNT = 0;
	NRF_SPIM1->EVENTS_END = 0x0UL;
	NRF_SPIM1->TASKS_START = 0x1UL;
	while(NRF_SPIM1->EVENTS_END == 0x0UL);
	// rx data
	NRF_SPIM1->RXD.PTR = (uint32_t)rx_buf;
	NRF_SPIM1->RXD.MAXCNT = rx_length;
	NRF_SPIM1->TXD.MAXCNT = 0;
	NRF_SPIM1->EVENTS_END = 0x0UL;
	NRF_SPIM1->TASKS_START = 0x1UL;
	while(NRF_SPIM1->EVENTS_END == 0x0UL);
	nrf_gpio_pin_set(cs_pin);
#endif
}

#pragma GCC optimize("O3")
inline static void dw_spiTx(uint32_t length, const uint8_t *buf) {
	NRF_SPIM0->TXD.PTR = (uint32_t)buf;
	NRF_SPIM0->TXD.MAXCNT = length;
	NRF_SPIM0->RXD.MAXCNT = 0;
	NRF_SPIM0->EVENTS_END = 0x0UL;
	NRF_SPIM0->TASKS_START = 0x1UL;
	while(NRF_SPIM0->EVENTS_END == 0x0UL);
}
#pragma GCC optimize("O3")
inline static void dw_spiRx(uint32_t length, uint8_t *buf) {
	NRF_SPIM0->RXD.PTR = (uint32_t)buf;
	NRF_SPIM0->RXD.MAXCNT = length;
	NRF_SPIM0->TXD.MAXCNT = 0;
	NRF_SPIM0->EVENTS_END = 0x0UL;
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
