/*
 * port_crc.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */
#include "port.h"
#include "crc16.h"

static volatile uint16_t CRC_SUM;

void PORT_CrcInit() {
  CRC_SUM = 0xFFFF;
}

void PORT_CrcReset() {
  CRC_SUM = 0xFFFF;
}

uint16_t PORT_CrcFeed(const void* bytes, int nBytes) {
	CRC_SUM = crc16_compute(bytes, nBytes, (uint16_t*)&CRC_SUM);
  return CRC_SUM;
}
