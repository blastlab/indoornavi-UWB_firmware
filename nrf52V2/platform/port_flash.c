/*
 * port_flash.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */
#include "port.h"
#include "FU.h"
#include "nrf.h"
#include "nrf_sdh.h"
#include "nrf_soc.h"
#include "string.h"

extern char PROG_SETTINGS;
#define APP_SETTINGS_ADDR	(uint32_t*)((void *)(&PROG_SETTINGS))			// defined in linker's script

static volatile bool flash_operation_ready;

static uint8_t fu_buf[4096];
static uint32_t *_flash_page_addr = NULL;

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
	PORT_WatchdogRefresh();
	uint32_t *dst = (uint32_t *)BOOTLOADER_MAGIC_REG_ADDR;
	uint32_t *src = (uint32_t *)&value;
	flash_operation_ready = false;
	sd_flash_write(dst, src, 1);
	while(!flash_operation_ready && nrf_sdh_is_enabled());
	PORT_WatchdogRefresh();
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

	if (flash_addr != APP_SETTINGS_ADDR) {
		_flash_page_addr = 0;
		memset(fu_buf, 0xff, FLASH_PAGE_SIZE);
	}
	return status;
}

static uint32_t save_to_flash(uint32_t * dst, uint32_t * src, uint32_t length) {
	uint32_t status = 0;
	int len = length;
	do {
		flash_operation_ready = false;
		int temp_len = (len >= FLASH_PAGE_SIZE) ? (FLASH_PAGE_SIZE / 4) : (len % FLASH_PAGE_SIZE / 4);
		status = sd_flash_write(dst, src, temp_len);
		do {
			PORT_WatchdogRefresh();
		} while(!flash_operation_ready && nrf_sdh_is_enabled());
		dst += FLASH_PAGE_SIZE/4;
		src += FLASH_PAGE_SIZE/4;
		len -= FLASH_PAGE_SIZE;
	} while(len > 0);
	/*while (status == 0 && memcmp(dst, src, length) != 0) {
	 }*/
	return status;
}

static uint32_t* ToPageStartAddr(const void* addr) {
	uint32_t ad = (uint32_t)addr;
	uint32_t* result = (uint32_t*)(ad - ad % FLASH_PAGE_SIZE);
	return result;
}

static int ItIsEotPacket(uint32_t* addr) {
	int result = addr == (FU_DESTINATION_1 + FLASH_PAGE_SIZE);
	result |= addr == (FU_DESTINATION_2 + FLASH_PAGE_SIZE);
	return result;
}

static int EotWillBeNextPacket(uint32_t* addr, int len) {
	uint32_t* ad = (uint32_t*)((int)addr + len);
	return ItIsEotPacket(ad);
}

// saving given number of bytes under choosen address
int PORT_FlashSave(void *destination, const void *p_source, uint32_t length) {
	uint32_t status = 0;
	uint32_t *dst = (uint32_t *)destination;
	uint32_t *dst_page = ToPageStartAddr(dst);
	uint32_t *src = (uint32_t *)p_source;
	uint32_t buf_ind = (uint32_t)dst % FLASH_PAGE_SIZE;
	PORT_ASSERT(((uint32_t)destination % 8) == 0);
	// return when same data is already saved i.e. after retransmission
	if (memcmp(dst, p_source, length) == 0) {
		return PORT_Success;
	}

	if (dst != APP_SETTINGS_ADDR) {							// if we are not saving current app's settings
		if (_flash_page_addr != 0 && _flash_page_addr != dst_page
		    && ToPageStartAddr((uint32_t*)((int)dst + length)) == dst_page) {	// if the addr_buffer changed
			status = save_to_flash(_flash_page_addr, (uint32_t *)fu_buf, FLASH_PAGE_SIZE);
			memset(fu_buf, 0xFF, FLASH_PAGE_SIZE);
			if (status != 0) {
				return status;
			}
		}

		if (ItIsEotPacket(dst)) {		// if this is EOT packet from FU
			memcpy(fu_buf, dst, FLASH_PAGE_SIZE);
			memcpy(fu_buf, src, length);
			status = PORT_FlashErase(dst, FLASH_PAGE_SIZE);
			if(!status) {
				status = save_to_flash(dst, (uint32_t *)fu_buf, FLASH_PAGE_SIZE);
			}
			return status;
		} else {
			_flash_page_addr = dst_page;												// if this is another packet within page
			if (ToPageStartAddr((uint32_t*)((int)dst + length)) != dst_page) {
				// gdy paczka zaczyna sie w 1. stronie flash a konczy w innej
				// lub jest ostatnia paczka w na tej stronie
				// uzupelnij strone do konca, zapisz flash i
				int lenInFirst = (int)dst_page + FLASH_PAGE_SIZE - (int)dst;
				memcpy(&fu_buf[buf_ind], src, lenInFirst);
				status = save_to_flash(dst_page, (uint32_t *)fu_buf, FLASH_PAGE_SIZE);
				memset(fu_buf, 0xFF, FLASH_PAGE_SIZE);
				if (status == 0) {
					memcpy(&fu_buf[0], (uint32_t*)((int)src + lenInFirst), length - lenInFirst);
					_flash_page_addr = ToPageStartAddr(dst + length);
				}
			} else {
				memcpy(&fu_buf[buf_ind], src, length);
			}
			return PORT_Success;
		}
	}
	status = save_to_flash(dst, src, length);
	return status;
}
