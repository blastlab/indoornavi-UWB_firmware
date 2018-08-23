#ifndef _PORT_CONST_H
#define _PORT_CONST_H

#include "decadriver/deca_device_api.h" // decaIrqStatus_t
#include <stdbool.h>
#include <stdint.h>
#include "nrf_error.h"
#include "nordic_common.h"

#define __H_MAJOR__ 0
#define __H_MINOR__ 1
#define HARDWARE_UID_64 (*(uint64_t *)(0x1FFF7590))
#define HARDWARE_OTP_ADDR 0x1FFF7000					// TODO: set OTP address

#include "iassert.h"
#define PORT_ASSERT(expr) IASSERT(expr)
#define UNUSED(x) ((void)(x))

extern char PROG_FLASH_START;
extern char PROG_FLASH_SIZE;
extern char PROG_PAGE_SIZE;
extern char PROG_BOOTLOADER_MAX_SIZE;

#define FLASH_BASE						(uint32_t)((void *)(&PROG_FLASH_START))			// defined in linker's script
#define FLASH_BANK_SIZE  				(uint32_t)((void *)(&PROG_FLASH_SIZE))			// flash size (with SD excluded)
#define FLASH_PAGE_SIZE					(uint32_t)((void *)(&PROG_PAGE_SIZE))
#define PROG_BOOTLOADER_MAX_SIZE_m		(uint32_t)((void *)(&PROG_BOOTLOADER_MAX_SIZE))

#define BOOTLOADER_MAGIC_NUMBER 		(0xBECA95)
#define BOOTLOADER_MAGIC_REG_ADDR		(uint32_t)(FLASH_BASE + PROG_BOOTLOADER_MAX_SIZE_m - 2*FLASH_PAGE_SIZE)
#define BOOTLOADER_MAGIC_REG			(uint32_t *)BOOTLOADER_MAGIC_REG_ADDR
#define BOOTLOADER_BKP_REG_ADDR			(uint32_t)(FLASH_BASE + PROG_BOOTLOADER_MAX_SIZE_m - 1*FLASH_PAGE_SIZE)
#define STATUS_MAGIC_REG				(uint32_t *)BOOTLOADER_BKP_REG_ADDR
#define STATUS_MAGIC_NUMBER_GO_SLEEP 	(0x12345678)

#define PORT_Success NRF_SUCCESS

#define LOG_USB_EN 1
#define LOG_SD_EN 0
#define LOG_USB_UART 0

// define how many high resolution clock tick is in one us
#define PORT_TICKS_HR_PER_US 1000

#define USE_DECA_DEVKIT		1

#if USE_DECA_DEVKIT
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
#define LED_STAT LED_G1
#define LED_ERR LED_R1
#else
#define DW_RST_PIN			24
#define DW_EXTI_IRQn 		19

#define DW_SPI_SS_PIN		17
#define DW_SPI_MISO_PIN		18
#define DW_SPI_MOSI_PIN		20
#define DW_SPI_SCK_PIN		16

#define USB_UART_RX_PIN 	11
#define USB_UART_TX_PIN 	5

// leds
#define LED_G1 2
#define LED_R1 15
#define LED_STAT LED_G1
#define LED_ERR LED_R1
#endif

#endif
