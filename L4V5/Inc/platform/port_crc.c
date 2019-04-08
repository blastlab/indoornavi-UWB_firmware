/*
 * port_crc.c
 *
 *  Created on: 25.03.2018
 *      Author: KarolTrzcinski
 */
#include "platform/port.h"

void PORT_CrcInit() {
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_CRC);
	LL_CRC_SetPolynomialCoef(CRC, 0x1021);
	LL_CRC_SetPolynomialSize(CRC, LL_CRC_POLYLENGTH_16B);
}

uint16_t PORT_CrcFeed(uint16_t * crc, const void* bytes, int nBytes) {
	const uint8_t* dPtr = (uint8_t*)bytes;
	LL_CRC_SetInitialData(CRC, *crc);
	for (; nBytes > 3; dPtr += 4, nBytes -= 4) {
		LL_CRC_FeedData32(CRC, (uint32_t)(dPtr[0]) << 24 | dPtr[1] << 16 | dPtr[2] << 8 | dPtr[3]);
	}
	for (; nBytes > 0; ++dPtr, --nBytes) {
		LL_CRC_FeedData8(CRC, *dPtr);
	}
	*crc = LL_CRC_ReadData16(CRC);
	return *crc;
}
