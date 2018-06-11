/*
 * port_spi.c
 *
 *  Created on: 21.03.2018
 *      Author: KarolTrzcinski
 */

#include "port.h"

#define DW_SPI_LL 0

// ==== SPI ====
#define DW_SPI_F_HI 20000000
#define DW_SPI_F_LO 3000000
// only for L4
//#define DW_SPI SPI1 TODO: implement SPI handler
//#define DW_SPI_LL // remember it is slow
#if DW_SPI_LL
#define DW_DMA DMA1
#define DW_DMACH_RX LL_DMA_CHANNEL_2
#define DW_DMACH_TX LL_DMA_CHANNEL_3
#else
//#define DW_DMACH_RX DMA1_Channel2 TODO: implement dma channels handlers
//#define DW_DMACH_TX DMA1_Channel3
#endif

void PORT_SpiSpeedSlow(bool slow) {

}

#if DW_SPI_LL

void api_init() {
  const uint32_t conf = LL_DMA_MODE_NORMAL | LL_DMA_PRIORITY_VERYHIGH |
                        LL_DMA_PERIPH_NOINCREMENT | LL_DMA_MEMORY_INCREMENT |
                        LL_DMA_PDATAALIGN_BYTE | // peripherial data size
                        LL_DMA_MDATAALIGN_BYTE;  // memory data size

  LL_DMA_ConfigTransfer(DW_DMA, DW_DMACH_TX,
                        LL_DMA_DIRECTION_MEMORY_TO_PERIPH | conf);
  LL_DMA_ConfigTransfer(DW_DMA, DW_DMACH_RX,
                        LL_DMA_DIRECTION_PERIPH_TO_MEMORY | conf);

  // set SPI address
  LL_DMA_SetPeriphAddress(DW_DMA, DW_DMACH_TX,
                          (uint32_t)LL_SPI_DMA_GetRegAddr(DW_SPI));
  LL_DMA_SetPeriphAddress(DW_DMA, DW_DMACH_RX,
                          (uint32_t)LL_SPI_DMA_GetRegAddr(DW_SPI));

  // permanently connect SPI TX with DMA
  LL_SPI_EnableDMAReq_TX(DW_SPI);

  // spi threshold buf size
  LL_SPI_SetRxFIFOThreshold(DW_SPI, LL_SPI_RX_FIFO_TH_QUARTER);
}

#pragma GCC optimize("O3")
static inline void spi_tx(uint32_t length, const uint8_t *buf) {
  // configure buffer address and length
  LL_DMA_SetDataLength(DW_DMA, DW_DMACH_TX, length);
  LL_DMA_SetMemoryAddress(DW_DMA, DW_DMACH_TX, (uint32_t)buf);

  // enable SPI TX
  LL_DMA_EnableChannel(DW_DMA, DW_DMACH_TX); // set DMA_CCR_EN
  LL_SPI_Enable(DW_SPI);

  // wait for end of transmission
  while (LL_DMA_GetDataLength(DW_DMA, DW_DMACH_TX)) {
  }
  while (LL_SPI_IsActiveFlag_BSY(DW_SPI)) {
  }

  LL_DMA_DisableChannel(DW_DMA, DW_DMACH_TX);
}

#pragma GCC optimize("O3")
static inline void spi_rx(uint32_t length, uint8_t *buf) {
  // configure buffer address and length
  // TX buf isn't important so we doesn't change it
  LL_DMA_SetDataLength(DW_DMA, DW_DMACH_TX, length); // set CNDTR
  LL_DMA_SetDataLength(DW_DMA, DW_DMACH_RX, length);
  LL_DMA_SetMemoryAddress(DW_DMA, DW_DMACH_RX, (uint32_t)buf);

  // read dummy byte and clear RXNE flag
  while (DW_SPI->SR & SPI_SR_FRLVL_Msk) {
    DW_SPI->DR;
  }

  // enable SPI RX
  LL_SPI_EnableDMAReq_RX(DW_SPI);            // set SPI_CR2_RXDMAEN
  LL_DMA_EnableChannel(DW_DMA, DW_DMACH_RX); // set DMA_CCR_EN
  // enable SPI TX
  LL_DMA_EnableChannel(DW_DMA, DW_DMACH_TX);

  // wait for end of transmission
  while (LL_DMA_GetDataLength(DW_DMA, DW_DMACH_RX)) {
  }
  while (LL_SPI_IsActiveFlag_BSY(DW_SPI)) {
  }

  // disable streams
  LL_DMA_DisableChannel(DW_DMA, DW_DMACH_RX);
  LL_DMA_DisableChannel(DW_DMA, DW_DMACH_TX);

  // disconnect DMA RX from SPI
  LL_SPI_DisableDMAReq_RX(DW_SPI);
}

#pragma GCC optimize("O3")
int spi_write(uint16_t hdrLength, const uint8_t *hdrBuf, uint32_t bodyLength,
              const uint8_t *bodyBuf) {
  decaIrqStatus_t en = decamutexon();
  LL_GPIO_ResetOutputPin(DW_CS_GPIO_Port, DW_CS_Pin);

  // send header and body buffer
  spi_tx(hdrLength, hdrBuf);
  spi_tx(bodyLength, bodyBuf);

  LL_GPIO_SetOutputPin(DW_CS_GPIO_Port, DW_CS_Pin);
  decamutexoff(en);
  return 0;
}

#pragma GCC optimize("O3")
int spi_read(uint16_t headerLength, const uint8_t *headerBuffer,
             uint32_t bodyLength, uint8_t *bodyBuffer) {
  decaIrqStatus_t en = decamutexon();
  LL_GPIO_ResetOutputPin(DW_CS_GPIO_Port, DW_CS_Pin);

  // send header and body buffer
  spi_tx(headerLength, headerBuffer);
  spi_rx(bodyLength, bodyBuffer);

  LL_GPIO_SetOutputPin(DW_CS_GPIO_Port, DW_CS_Pin);
  decamutexoff(en);
  return 0;
}

void PORT_WakeupTransceiver(void)
{
	LL_GPIO_ResetOutputPin(DW_CS_GPIO_Port, DW_CS_Pin);
	PORT_SleepMs(1);
	LL_GPIO_SetPinOutputType(DW_CS_GPIO_Port, DW_CS_Pin);
	PORT_SleepMs(7); //wait 7ms for DW1000 XTAL to stabilise
}

#else

void PORT_SpiInit() {

}

#pragma GCC optimize("O3")
static inline void spi_tx(uint32_t length, const uint8_t *buf) {

}

#pragma GCC optimize("O3")
static inline void spi_rx(uint32_t length, uint8_t *buf) {

}

#pragma GCC optimize("O3")
int readfromspi(uint16_t headerLength, const uint8_t *headerBuffer,
                uint32_t bodyLength, uint8_t *bodyBuffer) {
return 0;
}

#pragma GCC optimize("O3")
int writetospi(uint16_t headerLength, const uint8_t *headerBuffer,
               uint32_t bodyLength, const uint8_t *bodyBuffer) {

  return 0;
}

void PORT_WakeupTransceiver(void)
{

}
#endif
