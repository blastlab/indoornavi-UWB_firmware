/*
 * port_crc.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */
#include <stdio.h>
#include "port.h"
#include "crc16.h"
#include "crc32.h"

static uint32_t CRC_SUM;

void PORT_CrcInit()
{

}

void PORT_CrcReset()
{
	CRC_SUM = 0xFFFF;
}
#include "string.h"
uint16_t PORT_CrcFeed(const void *data, int size)
{
	const uint8_t *dPtr = (uint8_t*)data;
	for (; size > 3; dPtr += 4, size -= 4) {
	  uint32_t val = (uint32_t)(dPtr[0] << 24 | dPtr[1] << 16 | dPtr[2] << 8 | dPtr[3]);
	  CRC_SUM = crc32_compute((const uint8_t *)&val, 4, CRC_SUM == 0xFFFF ? NULL : &CRC_SUM);
	}
	for (; size > 0; dPtr += 2, size -= 2) {
		uint16_t val = (uint16_t)(dPtr[0] << 8 | dPtr[1]);
		CRC_SUM = crc16_compute((const uint8_t *)&val, 2, (uint16_t *)&CRC_SUM);
	}

	return 0;
//	return CRC_SUM;
}
