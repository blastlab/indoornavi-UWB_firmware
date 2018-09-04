/*
 * port_crc.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */
#include <stdio.h>
#include "port.h"

typedef uint16_t crc;
#define WIDTH  (8 * sizeof(crc))
#define TOPBIT (1 << (WIDTH - 1))
#define POLYNOMIAL 0x1DB7

static uint32_t CRC_SUM;
crc  crcTable[256];

void PORT_CrcInit() {
	crc remainder;
	for (int dividend = 0; dividend < 256; ++dividend) {
		remainder = dividend << (WIDTH - 8);
		for (uint8_t bit = 8; bit > 0; --bit) {
			if (remainder & TOPBIT) {
				remainder = (remainder << 1) ^ POLYNOMIAL;
			} else {
				remainder = (remainder << 1);
			}
		}
		crcTable[dividend] = remainder;
	}
	CRC_SUM = 0xFFFF;
}

void PORT_CrcReset() {
	CRC_SUM = 0xFFFF;
}

uint16_t PORT_CrcFeed(const void *bytes, int nBytes) {
	const uint8_t *message = (uint8_t *)bytes;
	uint8_t data;
	crc remainder = CRC_SUM;
	for (int byte = 0; byte < nBytes; ++byte) {
		data = message[byte] ^ (remainder >> (WIDTH - 8));
		remainder = crcTable[data] ^ (remainder << 8);
	}
	CRC_SUM = remainder;
	return CRC_SUM;
}
