/*
 * port_crc.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */
#include "port.h"
#include "crc16.h"

uint16_t PORT_CrcFeed(uint16_t * crc, const void* bytes, int nBytes) {
	*crc = crc16_compute(bytes, nBytes, crc);
	return *crc;
}
