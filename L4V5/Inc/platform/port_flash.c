/*
 * port_flash.c
 *
 *  Created on: 21.03.2018
 *      Author: KarolTrzcinski
 */
#include "port.h"


// czysci rejon strony flasha pod nowy firmware
HAL_StatusTypeDef port_flash_erase(void *flash_addr, uint32_t length) {
	PORT_ASSERT((uint32_t)flash_addr >= FLASH_BASE);
	PORT_ASSERT(length < FLASH_BANK_SIZE);
	HAL_StatusTypeDef ret = HAL_OK;
	FLASH_EraseInitTypeDef feitd;
	uint32_t pageError;
	int nPages;
	// fill flash erase struct
	feitd.TypeErase = FLASH_TYPEERASE_PAGES;
	feitd.Banks = FLASH_BANK_1;
	feitd.NbPages = 1;
	feitd.Page = ((uint32_t)flash_addr-FLASH_BASE) / FLASH_PAGE_SIZE;

	// calc number of pages
	nPages = length / FLASH_PAGE_SIZE;
	if(nPages*FLASH_PAGE_SIZE < length) {
		++nPages;
	}

	// erase flash
	HAL_FLASH_Unlock();
	port_watchdog_refresh();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
	while(ret == HAL_OK && nPages > 0) {
		ret = HAL_FLASHEx_Erase(&feitd, &pageError); // up to 25ms
		port_watchdog_refresh(); //necessarily refresh watchdog after page erase
		--nPages;
		++feitd.Page;
	}
	HAL_FLASH_Lock();

	return ret;
}

// zapisuje ilosc bajtow we flashu pod wskazany adres, length % 8 musi byc rowne 0
HAL_StatusTypeDef port_flash_save(void* destination, const void *p_source, uint32_t length){
	uint8_t status = 0;
	uint32_t i = 0;
	uint8_t* dst = (uint8_t*)destination;
	uint32_t* src = (uint32_t*)p_source;
	PORT_ASSERT((uint32_t)destination%8 == 0);
	// gdy te dane sa juz tam zapisane
	// np podczas retransmisji danych
	if(memcmp(dst, p_source, length) == 0)
	{
		return HAL_OK;
	}
	__disable_irq();
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
	// DataLength must be a multiple of 64 bit
	for (i = 0; i < length; i+=8, dst += 8, src+=2){ // dst and i is in bytes and p_source in uint32_t
		// Device voltage range supposed to be [2.7V to 3.6V], the operation will be done by word
		uint64_t t;
		*(uint32_t*)&t = *src;
		*(((uint32_t*)&t)+1) = *(src+1);
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)dst, t) == HAL_OK){
			if (*(uint64_t*)dst != t) {// Check the written value
				status = 2; // Flash content doesn't match SRAM content
				break;
			}
		} else {
			status = 1; // Error occurred while writing data in Flash memory
			status = HAL_FLASH_GetError();
			break;
		}
	}
	HAL_FLASH_Lock();
	__enable_irq();
	return status;
}
