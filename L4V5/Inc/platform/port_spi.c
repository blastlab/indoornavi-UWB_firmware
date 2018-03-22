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
#define DW_SPI SPI1
//#define DW_SPI_LL // remember it is slow
#if DW_SPI_LL
#define DW_DMA DMA1
#define DW_DMACH_RX LL_DMA_CHANNEL_2
#define DW_DMACH_TX LL_DMA_CHANNEL_3
#else
#define DW_DMACH_RX DMA1_Channel2
#define DW_DMACH_TX DMA1_Channel3
#endif

void spi_speed_slow(bool slow) {
  if (!slow) {
    LL_SPI_SetBaudRatePrescaler(DW_SPI, LL_SPI_BAUDRATEPRESCALER_DIV2);
  } else {
    LL_SPI_SetBaudRatePrescaler(DW_SPI, LL_SPI_BAUDRATEPRESCALER_DIV16);
  }
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

#else

void spi_init() {
  const uint32_t conf = LL_DMA_MODE_NORMAL | LL_DMA_PRIORITY_VERYHIGH |
                        LL_DMA_PERIPH_NOINCREMENT | LL_DMA_MEMORY_INCREMENT |
                        LL_DMA_PDATAALIGN_BYTE | // peripherial data size
                        LL_DMA_MDATAALIGN_BYTE;  // memory data size

  // LL_DMA_ConfigTransfer(DW_DMA, DW_DMACH_TX,
  // LL_DMA_DIRECTION_MEMORY_TO_PERIPH|conf);
  MODIFY_REG(DW_DMACH_TX->CCR,
             DMA_CCR_DIR | DMA_CCR_MEM2MEM | DMA_CCR_CIRC | DMA_CCR_PINC |
                 DMA_CCR_MINC | DMA_CCR_PSIZE | DMA_CCR_MSIZE | DMA_CCR_PL,
             LL_DMA_DIRECTION_MEMORY_TO_PERIPH | conf);

  MODIFY_REG(DW_DMACH_RX->CCR,
             DMA_CCR_DIR | DMA_CCR_MEM2MEM | DMA_CCR_CIRC | DMA_CCR_PINC |
                 DMA_CCR_MINC | DMA_CCR_PSIZE | DMA_CCR_MSIZE | DMA_CCR_PL,
             LL_DMA_DIRECTION_PERIPH_TO_MEMORY | conf);

  // set SPI address
  DW_DMACH_TX->CPAR = DW_DMACH_RX->CPAR = (uint32_t)&DW_SPI->DR;

  // permanently connect SPI TX with DMA
  LL_SPI_EnableDMAReq_TX(DW_SPI);

  // spi threshold buf size
  LL_SPI_SetRxFIFOThreshold(DW_SPI, LL_SPI_RX_FIFO_TH_QUARTER);
}

#pragma GCC optimize("O3")
static inline void spi_tx(uint32_t length, const uint8_t *buf) {
  // configure buffer address and length
  DW_DMACH_TX->CMAR = (uint32_t)
      buf; // LL_DMA_SetMemoryAddress(DW_DMA, DW_DMACH_TX, (uint32_t)hdrBuf)
  DW_DMACH_TX->CNDTR =
      length; // LL_DMA_SetDataLength(DW_DMA, DW_DMACH_TX, hdrLength)

  // start transmission
  SET_BIT(DW_DMACH_TX->CCR, DMA_CCR_EN);

  // wait for end of transmission
  while (DW_DMACH_TX->CNDTR != 0) {
  }
  while (DW_SPI->SR & SPI_SR_BSY) {
  }

  // disable dma channel
  CLEAR_BIT(DW_DMACH_TX->CCR,
            DMA_CCR_EN); // LL_DMA_DisableChannel(DW_DMA, DW_DMACH_TX);
}

#pragma GCC optimize("O3")
static inline void spi_rx(uint32_t length, uint8_t *buf) {
  // configure buffer address and length
  DW_DMACH_RX->CMAR = (uint32_t)
      buf; // LL_DMA_SetMemoryAddress(DW_DMA, DW_DMACH_RX, (uint32_t)buf)
  DW_DMACH_RX->CNDTR =
      length; // LL_DMA_SetDataLength(DW_DMA, DW_DMACH_RX, length)
  DW_DMACH_TX->CNDTR =
      length; // LL_DMA_SetDataLength(DW_DMA, DW_DMACH_TX, length)

  // read dummy bytes
  while (DW_SPI->SR & SPI_SR_RXNE) {
    DW_SPI->DR;
  }

  // connect SPI rx with DMA
  SET_BIT(DW_SPI->CR2, SPI_CR2_RXDMAEN); // LL_SPI_EnableDMAReq_RX(DW_SPI);
  // start transmission
  SET_BIT(DW_DMACH_RX->CCR,
          DMA_CCR_EN); // LL_DMA_EnableChannel(DW_DMA, DW_DMACH_RX);
  SET_BIT(DW_DMACH_TX->CCR,
          DMA_CCR_EN); // LL_DMA_EnableChannel(DW_DMA, DW_DMACH_TX);

  // wait for end of transmission
  while (DW_DMACH_RX->CNDTR != 0) {
  }
  while (DW_SPI->SR & SPI_SR_BSY) {
  }

  // disable dma channels
  CLEAR_BIT(DW_DMACH_RX->CCR,
            DMA_CCR_EN); // LL_DMA_DisableChannel(DW_DMA, DW_DMACH_RX);
  CLEAR_BIT(DW_DMACH_TX->CCR,
            DMA_CCR_EN); // LL_DMA_DisableChannel(DW_DMA, DW_DMACH_TX);
  // disconnect rx dma from SPI
  CLEAR_BIT(DW_SPI->CR2, SPI_CR2_RXDMAEN); // LL_SPI_DisableDMAReq_RX(DW_SPI);
}

#pragma GCC optimize("O3")
int readfromspi(uint16_t headerLength, const uint8_t *headerBuffer,
                uint32_t bodyLength, uint8_t *bodyBuffer) {
  decaIrqStatus_t en = decamutexon();
  DW_CS_GPIO_Port->BRR =
      DW_CS_Pin; // LL_GPIO_ResetOutputPin(DW_CS_GPIO_Port, DW_CS_Pin)

  SET_BIT(DW_SPI->CR1, SPI_CR1_SPE); // LL_SPI_Enable(DW_SPI);
  spi_tx(headerLength, headerBuffer);
  spi_rx(bodyLength, bodyBuffer);
  CLEAR_BIT(DW_SPI->CR1, SPI_CR1_SPE); // LL_SPI_Disable(DW_SPI):

  DW_CS_GPIO_Port->BSRR =
      DW_CS_Pin; // LL_GPIO_SetOutputPin(DW_CS_GPIO_Port, DW_CS_Pin)
  decamutexoff(en);
  return 0;
}

#pragma GCC optimize("O3")
int writetospi(uint16_t headerLength, const uint8_t *headerBuffer,
               uint32_t bodyLength, const uint8_t *bodyBuffer) {
  decaIrqStatus_t en = decamutexon();
  DW_CS_GPIO_Port->BRR =
      DW_CS_Pin; // LL_GPIO_ResetOutputPin(DW_CS_GPIO_Port, DW_CS_Pin)

  SET_BIT(DW_SPI->CR1, SPI_CR1_SPE); // LL_SPI_Enable(DW_SPI);
  spi_tx(headerLength, headerBuffer);
  spi_tx(bodyLength, bodyBuffer);
  CLEAR_BIT(DW_SPI->CR1, SPI_CR1_SPE); // LL_SPI_Disable(DW_SPI):

  DW_CS_GPIO_Port->BSRR =
      DW_CS_Pin; // LL_GPIO_SetOutputPin(DW_CS_GPIO_Port, DW_CS_Pin)
  decamutexoff(en);
  return 0;
}

#endif
