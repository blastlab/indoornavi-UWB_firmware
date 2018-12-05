#ifndef _PORT_CONST_H
#define _PORT_CONST_H

#include <stdbool.h>
#include <stdint.h>
#include "nordic_common.h"
#include "nrf52.h"
#include "nrf_error.h"

#define HARDWARE_UID_64 (((uint64_t)NRF_FICR->DEVICEADDR[1] << 32) | NRF_FICR->DEVICEADDR[0])
#define __H_MAJOR__ 2
#define __H_MINOR__ 2

#include "iassert.h"
#define PORT_ASSERT(expr) IASSERT(expr)
#define UNUSED(x) ((void)(x))

extern char PROG_FLASH_START;
extern char PROG_FLASH_SIZE;
extern char PROG_PAGE_SIZE;
extern char PROG_BOOTLOADER_MAX_SIZE;

#define FLASH_BASE \
  (uint32_t)((void*)(&PROG_FLASH_START))  // defined in linker's script
#define FLASH_BANK_SIZE \
  (uint32_t)((void*)(&PROG_FLASH_SIZE))  // flash size (with SD excluded)
#define FLASH_PAGE_SIZE (uint32_t)((void*)(&PROG_PAGE_SIZE))
#define PROG_BOOTLOADER_MAX_SIZE_m \
  (uint32_t)((void*)(&PROG_BOOTLOADER_MAX_SIZE))

#define BOOTLOADER_MAGIC_NUMBER (0xBECA95)
#define BOOTLOADER_MAGIC_REG_ADDR \
  (uint32_t)(FLASH_BASE + PROG_BOOTLOADER_MAX_SIZE_m - 2 * FLASH_PAGE_SIZE)
#define BOOTLOADER_MAGIC_REG (uint32_t*)BOOTLOADER_MAGIC_REG_ADDR
#define BOOTLOADER_BKP_REG_ADDR \
  (uint32_t)(FLASH_BASE + PROG_BOOTLOADER_MAX_SIZE_m - 1 * FLASH_PAGE_SIZE)
#define STATUS_MAGIC_REG (uint32_t*)BOOTLOADER_BKP_REG_ADDR
#define STATUS_MAGIC_NUMBER_GO_SLEEP (0x12345678)

#define PORT_Success NRF_SUCCESS

#define LOG_USB_EN 1
#define LOG_SPI_EN 0
#define LOG_SD_EN 0
#define LOG_USB_UART 0

// define how many high resolution clock tick is in one us
#define PORT_TICKS_HR_PER_US 1000

typedef uint32_t time_ms_t;

#define USE_BLE 0

#if USE_BLE
#define BLE_CODE(_CODE_) \
  { _CODE_ }
#else
#define BLE_CODE(_CODE_)
#endif

#define ETH_SINK_PCB 0
#define TAG_PCB 0

#if ETH_SINK_PCB
#undef LOG_SPI_EN
#define LOG_SPI_EN 1

#define ETH_SPI_SLAVE_IRQ 0

#define DW_RST_PIN 24
#define DW_EXTI_IRQn 19
#define DW_SPI_MISO_PIN 18
#define DW_SPI_MOSI_PIN 20
#define DW_SPI_SCK_PIN 16
#define DW_SPI_SS_PIN 17

#define GEN_SPI_MISO_PIN 7
#define GEN_SPI_MOSI_PIN 6
#define GEN_SPI_SCK_PIN 4

#define ETH_SPI_SS_PIN 22

#define USB_UART_RX_PIN 11
#define USB_UART_TX_PIN 5

#define BATT_ADC_TRIG_PIN 15	// add pull transistor on the board!
#define HW_TYPE_PULL 1

#define LED_G1 0
#define LED_R1 0
#elif TAG_PCB
#define DW_RST_PIN 24
#define DW_EXTI_IRQn 19
#define DW_SPI_MISO_PIN 18
#define DW_SPI_MOSI_PIN 20
#define DW_SPI_SCK_PIN 16
#define DW_SPI_SS_PIN 17

#define GEN_SPI_MISO_PIN 7
#define GEN_SPI_MOSI_PIN 6
#define GEN_SPI_SCK_PIN 4

#define IMU_SPI_SS_PIN 3
#define IMU_EXTI_IRQ1 23
#define IMU_EXTI_IRQ2 13

#define USB_UART_RX_PIN 11
#define USB_UART_TX_PIN 5

#define BATT_ADC_TRIG_PIN 27
#define HW_TYPE_PULL 26

#define LED_G1 9
#define LED_R1 12
#else
#define DW_RST_PIN 24
#define DW_EXTI_IRQn 19
#define DW_SPI_MISO_PIN 18
#define DW_SPI_MOSI_PIN 20
#define DW_SPI_SCK_PIN 16
#define DW_SPI_SS_PIN 17

#define GEN_SPI_MISO_PIN 7
#define GEN_SPI_MOSI_PIN 6
#define GEN_SPI_SCK_PIN 4

#define IMU_SPI_SS_PIN 3
#define IMU_EXTI_IRQ1 23
#define IMU_EXTI_IRQ2 13

#define USB_UART_RX_PIN 11
#define USB_UART_TX_PIN 5

#define BATT_ADC_TRIG_PIN 27
#define HW_TYPE_PULL 10

#define LED_G1 2
#define LED_R1 15
#endif

#define LED_STAT LED_G1
#define LED_ERR LED_R1


#endif
