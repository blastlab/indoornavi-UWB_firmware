/*
 * port_flash.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */
#include "port.h"
#include "nrf.h"
#include "nrf_sdh.h"
#include "nrf_soc.h"
#include "string.h"


static volatile bool flash_operation_ready;
void soc_evt_handler(uint32_t evt_id, void * p_context) {
	switch(evt_id)
	{
		case NRF_EVT_FLASH_OPERATION_SUCCESS:
			flash_operation_ready = true;
			break;

		case NRF_EVT_FLASH_OPERATION_ERROR:
			IASSERT(0);
			break;
	}
}

// save value in reset-safe backup register
void PORT_BkpRegisterWrite(uint32_t *reg, uint32_t value)
{
	PORT_ASSERT((uint32_t)reg >= (uint32_t)BOOTLOADER_MAGIC_REG_ADDR);
	if(value == *BOOTLOADER_MAGIC_REG)
		return;
	flash_operation_ready = false;
	sd_flash_page_erase((uint32_t)BOOTLOADER_MAGIC_REG_ADDR/FLASH_PAGE_SIZE);
	while(!flash_operation_ready && nrf_sdh_is_enabled());
	uint32_t *dst = (uint32_t *)BOOTLOADER_MAGIC_REG_ADDR;
	uint32_t *src = (uint32_t *)&value;
	flash_operation_ready = false;
	sd_flash_write(dst, src, 1);
	while(!flash_operation_ready && nrf_sdh_is_enabled());
}

// read value from reset-safe backup register
uint32_t PORT_BkpRegisterRead(uint32_t *reg)
{
	PORT_ASSERT((uint32_t)reg >= (uint32_t)BOOTLOADER_MAGIC_REG_ADDR);
	return *BOOTLOADER_MAGIC_REG;
}

// erasing area in a flash under given address
int PORT_FlashErase(void *flash_addr, uint32_t length) {
	uint32_t status = 0;
	PORT_ASSERT((uint32_t)flash_addr >= FLASH_BASE);
	PORT_ASSERT(length < FLASH_BANK_SIZE);
	// calc number of pages
	int nPages = length / FLASH_PAGE_SIZE;
	if (nPages * FLASH_PAGE_SIZE < length)
	{
		++nPages;
	}
	PORT_WatchdogRefresh();
	for(uint32_t i = 0; i < nPages && status == 0; i++) {
		flash_operation_ready = false;
		status = sd_flash_page_erase(((uint32_t)flash_addr / FLASH_PAGE_SIZE) + i);
		do {
			PORT_WatchdogRefresh();
		} while(!flash_operation_ready && nrf_sdh_is_enabled());
	}
	return status;
}

// saving given number of bytes under choosen address
int PORT_FlashSave(void *destination, const void *p_source, uint32_t length) {
	uint32_t status = 0;
	uint32_t *dst = (uint32_t *)destination;
	uint32_t *src = (uint32_t *)p_source;
	PORT_ASSERT(((uint32_t)destination % 8) == 0);
	// return when same data is already saved i.e. after retransmission
	if (memcmp(dst, p_source, length) == 0) {
		return PORT_Success;
	}
	int len = length;
	do {
		flash_operation_ready = false;
		status = sd_flash_write(dst, src, (len >= FLASH_PAGE_SIZE) ? (FLASH_PAGE_SIZE / 4) : (len % FLASH_PAGE_SIZE / 4));
		do {
			PORT_WatchdogRefresh();
		} while(!flash_operation_ready && nrf_sdh_is_enabled());
		dst += FLASH_PAGE_SIZE/4;
		src += FLASH_PAGE_SIZE/4;
		len -= FLASH_PAGE_SIZE;
	} while(len > 0);
	return status;
}
