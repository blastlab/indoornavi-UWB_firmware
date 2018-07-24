/*
 * port_flash.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */
#include "port.h"
#include "nrf.h"
#include "nrf_nvmc.h"
#include "string.h"

// save value in reset-safe backup register
void PORT_BkpRegisterWrite(uint32_t *reg, uint32_t value)
{
  PORT_ASSERT((uint32_t)reg >= (uint32_t)&NRF_POWER->GPREGRET);
  *reg = value;
}

// read value from reset-safe backup register
uint32_t PORT_BkpRegisterRead(uint32_t *reg)
{
  PORT_ASSERT((uint32_t)reg >= (uint32_t)&NRF_POWER->GPREGRET);
  return *reg;
}

// erasing area in a flash under given address
int PORT_FlashErase(void *flash_addr, uint32_t length) {
	PORT_ASSERT((uint32_t)flash_addr >= FLASH_BASE);
	PORT_ASSERT(length < FLASH_BANK_SIZE);
	// calc number of pages
	int nPages = length / FLASH_PAGE_SIZE;
	if (nPages * FLASH_PAGE_SIZE < length)
	{
		++nPages;
	}
	PORT_WatchdogRefresh();
	for(uint32_t i = 0; i < nPages; i++) {
		nrf_nvmc_page_erase((uint32_t)(flash_addr + i));
		PORT_WatchdogRefresh();                      // necessarily refresh watchdog after page erase
	}
	return PORT_Success;
}

// saving given number of bytes under choosen address
int PORT_FlashSave(void *destination, const void *p_source, uint32_t length) {
	uint8_t status = 0;
	uint8_t *dst = (uint8_t *)destination;
	uint32_t *src = (uint32_t *)p_source;
	PORT_ASSERT(((uint32_t)destination % 8) == 0);
	// return when same data is already saved i.e. after retransmission
	if (memcmp(dst, p_source, length) == 0) {
		return PORT_Success;
	}
	// Enable write
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
	// DataLength must be a multiple of 64 bit
	for (uint32_t i = 0; i < length; i += 4, dst += 4, src += 1) {
		((uint32_t *)dst)[0] = 0xFFFFFFFFUL & *src;
		while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
		if (*(uint32_t *)dst != *src) {             			// Check the written value
			status = 2; 									// Flash content doesn't match SRAM content
			break;
		}
	}
	// Disable write
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
	return status;
}
