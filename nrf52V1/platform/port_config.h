#ifndef _PORT_CONST_H
#define _PORT_CONST_H

#include "decadriver/deca_device_api.h" // decaIrqStatus_t
#include <stdbool.h>
#include <stdint.h>
#include "nrf_error.h"
#include "nrf52.h"

#define __H_MAJOR__ 0
#define __H_MINOR__ 1
#define HARDWARE_UID_64 (*(uint64_t *)(0x1FFF7590))
#define HARDWARE_OTP_ADDR 0x1FFF7000					// TODO: set OTP address

#include "iassert.h"
#define PORT_ASSERT(expr) IASSERT(expr)
#define UNUSED(x) ((void)(x))


//char PROG_DESTINATION1; // TODO should be defined in linker script
//char PROG_DESTINATION2;

#define FLASH_PAGE_SIZE 	NRF_FICR->CODEPAGESIZE
#define FLASH_BASE 			((uint32_t)0x0U)
#define FLASH_BANK_SIZE  	((uint32_t)0x80000U)

#define PORT_Success NRF_SUCCESS


#define LOG_USB_EN 1
#define LOG_SD_EN 0
#define LOG_USB_UART 0

// define how many high resolution clock tick is in one us
#define PORT_TICKS_HR_PER_US 1000

#define USE_DECA_DEVKIT
#define BEACON_MODE 		1

#ifdef USE_DECA_DEVKIT
#define DW_RST_PIN			24
#define DW_EXTI_IRQn 		19

#define DW_SPI_SS_PIN		17
#define DW_SPI_MISO_PIN		18
#define DW_SPI_MOSI_PIN		20
#define DW_SPI_SCK_PIN		16

#define USB_UART_RX_PIN 	11
#define USB_UART_TX_PIN 	5

// leds
#define LED_G1 30
#define LED_R1 22
#define LED_B1 31
#define LED_STAT LED_G1
#define LED_ERR LED_R1
#define LED_BLE LED_B1
#else
#define DW_RST_PIN			27
#define DW_EXTI_IRQn 		26

#define DW_SPI_SS_PIN		25
#define DW_SPI_MISO_PIN		23
#define DW_SPI_MOSI_PIN		24
#define DW_SPI_SCK_PIN		22

#define USB_UART_RX_PIN 	8
#define USB_UART_TX_PIN 	6

// leds
#define LED_G1 17//30
#define LED_R1 18//22
#define LED_B1 19//31
#define LED_STAT LED_G1
#define LED_ERR LED_R1
#define LED_BLE LED_B1
#endif

#define BOOTLOADER_MAGIC_NUMBER 		(0xBC)
#define BOOTLOADER_MAGIC_REG 			((uint32_t*)&NRF_POWER->GPREGRET)
#define STATUS_MAGIC_REG 				((uint32_t*)&NRF_POWER->GPREGRET2)
#define STATUS_MAGIC_NUMBER_GO_SLEEP 	(0xFC)


#endif
