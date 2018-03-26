/*
 * port_crc.c
 *
 *  Created on: 25.03.2018
 *      Author: KarolTrzcinski
 */
#include "port.h"

void crc_init()
{
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_CRC);
  LL_CRC_SetPolynomialCoef(CRC, 0x1DB7);
  LL_CRC_SetPolynomialSize(CRC, LL_CRC_POLYLENGTH_16B);
}

void port_crc_reset()
{
	LL_CRC_SetInitialData(CRC, 0xFFFF);
}

uint16_t port_crc_feed(const void *data, int size)
{
	const uint8_t *dPtr = (uint8_t*)data;
  for (; size > 3; dPtr += 4, size -= 4) {
    LL_CRC_FeedData32(CRC, (uint32_t)(dPtr[0]) << 24 | dPtr[1] << 16 |
                                       dPtr[2] << 8 | dPtr[3]);
  }
  for (; size > 0; ++dPtr, --size) {
    LL_CRC_FeedData8(CRC, *dPtr);
  }
  return LL_CRC_ReadData16(CRC);
}
