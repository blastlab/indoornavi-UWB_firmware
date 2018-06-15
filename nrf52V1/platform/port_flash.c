/*
 * port_flash.c
 *
 *  Created on: 14.06.2018
 *      Author: DawidPeplinski
 */
#include "port.h"

// save value in reset-safe backup register
void PORT_BkpRegisterWrite(uint32_t *reg, uint32_t value)
{

}

// read value from reset-safe backup register
uint32_t PORT_BkpRegisterRead(uint32_t *reg)
{
	return 0;
}

// czysci rejon strony flasha pod nowy firmware
int PORT_FlashErase(void *flash_addr, uint32_t length)
{
  return 0;
}

// zapisuje ilosc bajtow we flashu pod wskazany adres, length % 8 musi byc rowne
// 0
int PORT_FlashSave(void *destination, const void *p_source, uint32_t length)
{
	return 0;
}
