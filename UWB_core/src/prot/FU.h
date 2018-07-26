/**
 * @brief Firmware upgrade protocol
 * 
 * @file fu.h
 * @author Karol Trzcinski
 * @date 2018-06-29
 * 
 * # Firmware upgrade protocol
 * 
 * ## Goal
 * It is used to upgrade firmware over the air (OTA), USB or any other
 * media. Transfer layer is provided by carry module and carry implementation
 * is separated from this module.
 * 
 * 
 * ## Details
 * 
 * 
 * ### Overview
 * 
 * To work fine whole available flash memory should be divided into
 * partitions. Current implementation use separate partition for:
 * 
 *   - bootloader
 *   - firmware A
 *   - firmware B
 * 
 * Where bootloader should start after each system reset and detect correct
 * firmware to use. Firmware A and B is a place for target application code
 * and when one version is in use then second one is a place for the new one.
 * 
 * 
 * ### Bootloader
 * 
 * After startup bootloader should be first application to run. Then work 
 * algorithm looks like:
 * 
 * 1. Check reset source
 *   - power up or bootloader watchdog reset - go to next point
 *   - others - run last application
 * 1. Check firmware versions compability, if change detected then start
 *   verification process
 * 1. Save current status if changed
 * 1. Choose better firmware to start
 * 
 * Verification process start bootloader watchdog (with 8s period) and start 
 * new firmware execution. After Watchdog reset, value of special register
 * to check if this version run without problems. If this register value
 * is correct then mark firmware as verified, save this as last running app
 * and start this application without bootloader watchdog. When application
 * doesn't set correct value to special register then downscore it and set app
 * to run in a standard way.
 * 
 * 
 * ### Firmware upgrade routine
 * 
 * At the begin SOT frame should be transmitted from Host to Device
 * and Device should response ACK frame.
 * Then is series of Data/ACK frames. ACK is required after each data frame
 * On last data frame, Device response with EOT frame instead of ACK.
 * It's extra information that firmware upgrade was fully successful.
 * To start firmware upgrade 
 * 
 * 
 * #### SOT -- Start Of Transmission -- Host->Device
 * 
 * This frame should be sent at the beginning before frames with raw firmware
 * data. After receiving this frame, whole flash area for new firmware should
 * be erased.
 * 
 * Construction:
 * 
 * [FC:1][frameLen:1][opcode:1][version:4][firmwareCRC:2][size:4][frameCRC:2]
 * 
 * - frameLen 		- length of while frame, from address of frameLen
 * 									to the end of frame CRC, equal to 12.
 * - opcode 			- set to FU_OPCODE_SOT
 * - version			- [lowest::older] byte of new firmware version
 * - firmwareCRC 	- CRC calculated only from complete raw firmware data
 * 									calculated in this same way as for each frame
 * - size 				- big endian, size in bytes of complete raw firmware data
 * - frameCRC			-	16 bit of CRC calculated on data from frameLen to
 * 									the end of offset, based  on FU_CRC_POLY
 * 
 * 
 * #### Default/Data frame -- Host->Device
 * 
 * Frame with firmware file data
 * 
 * Construction:
 * 
 * [frameLen:1][opcode:1][version:1][offset:2][data:n][frameCRC:2]
 * 
 * - frameLen 		- length of while frame, from address of frameLen
 * 									to the end of frame CRC
 * - opcode 			- set to FU_OPCODE_DATA
 * - version			- version of new firmware
 * - offset			- offset in memory for transmitted data.
 *  								Offset is expressed in FU_BLOCK_SIZE unit
 * - data				- raw byte data, number of them can be readed as
 *                  frameLen-FU_PROT_HEAD_SIZE-2 (for CRC), number of data
 *                  should be  multiple of FU_BLOCK_SIZE
 * - frameCRC		-	16 bit of CRC calculated on data from frameLen
 * 									to the end of offset, based on FU_CRC_POLY
 * 
 * 
 * #### ACK -- Device->Host
 * 
 * Previously sent packet was correct and device ready to receive new frame
 * 
 * Construction:
 * 
 * [frameLen:1][opcode:1][version:1][extra:2][frameCRC:2]
 * - frameLen 		- length of while frame, from address of frameLen
 * 									to the end of frame CRC. Equal to 6
 * - opcode 			- set to FU_OPCODE_ACK
 * - version			- lowest byte of current firmware version
 * - extra				- this field shouldn't be modified in ACK frame
 * 
 * 
 * #### EOT -- End Of Transmission -- Host->Device
 * 
 * Last data frame. After reception of this frame data compability should be
 * checked. When everything is ok, then ACK response should be send and device
 * should reboot to start new firmware verification procedure.
 * 
 * Construction:
 * 
 * [frameLen:1][opcode:1][version:1][extra:2][frameCRC:2]
 * 
 * - frameLen 		- length of while frame, from address of frameLen
 * 									to the end of frame CRC. Equal to 6
 * - opcode 			- set to FU_OPCODE_EOT
 * - version			- lowest byte of current firmware version
 * - extra				- this field shouldn't be modified in
 * 									EOT frame
 * 
 * 
 * #### ABORT -- Device->Host
 * 
 * Error info. Firmware upgrade process can't be continued.
 * 
 * Construction:
 * 
 * [frameLen:1][opcode:1][version:1][errorCode:2][frameCRC:2]
 * 
 * - frameLen 		- length of while frame, from address of frameLen
 * 									to the end of frame CRC. Equal to 8
 * - opcode 			- set to FU_OPCODE_ABORT
 * - version			- lowest byte of current firmware version
 * - errorCode   - FU_ERR_code enumerate
 * 
 * 
 * #### ASK_VER -- Host->Device
 * 
 * Ask about response ACK frame with current version of software
 * 
 * Construction:
 * 
 * [frameLen:1][opcode:1][version:4][frameCRC:2]
 * - frameLen 		- length of while frame, from address of frameLen
 * 									to the end of frame CRC. Equal to 6
 * - opcode 			- set to FU_OPCODE_ASK_VER
 * - version			- BigEndian, lowest byte of current firmware version
 * 
 */

#ifndef FU_H_
#define FU_H_

#include <string.h> // memcpy, memcmp

#include "../mac/carry.h"
#include "../mac/mac_const.h"     // prot_packet_info_t
#include "../parsers/bin_const.h" // FC_FU
#include "gitversion.h"           // version minor/major
#include "iassert.h"
#include "platform/port.h"


#define FU_ASSERT(expre) IASSERT(expre)

// define place to load new firmware
extern char PROG_DESTINATION1; ///< from linker script, address of partition A
extern char PROG_DESTINATION2; ///< from linker script, address of partition B
extern char PROG_START1;  ///< from linker script, isr vector location (part A)
extern char PROG_SETTINGS1;  ///< from linker script, settings location (part A)
#define FU_DESTINATION_1 ((void *)(&PROG_DESTINATION1)) 
#define FU_DESTINATION_2 ((void *)(&PROG_DESTINATION2))
#define FU_MAX_PROGRAM_SIZE ((int)(FU_DESTINATION_2 - FU_DESTINATION_1))


/**
 * @brief place in address where current firmware version is stored
 * offsed from FLASH_BASE
 */
#define FU_VERSION_LOC (FLASH_PAGE_SIZE + 4)


/**
 * @brief max tx data in target device is 8 bytes (+2 for CRC)
 * but for source device it depends on data size in block
 * 
 */
#define FU_MAX_DATA_SIZE (8 + 2)
//#define FU_MAX_DATA_SIZE				(64+2)


/**
 * @brief size of one minimal piece of data in bytes
 * 
 */
#define FU_BLOCK_SIZE 16


/**
 * @brief raw firmware upgrade protocol struct
 * 
 * definition of field extra may vary depending on opcode
 */
typedef struct {
  uint8_t FC;       ///< funtion code FU
  uint8_t frameLen; ///< size in bytes from start of FU_prot address to last
                    ///< byte of CRC
  uint8_t opcode;   ///< operation code[lobyte] and protocol version [hibyte]
                    ///< @see macro with FU_OPCODE_ prefix
                    ///< @see SFU_MakeOpcode
  uint8_t hash;     ///< mix of hardware and software version in this packet
  uint16_t extra;   ///< BigEndian, optional 16 bit data field with value 
                    ///< depending from opcode
  uint8_t data[FU_MAX_DATA_SIZE]; ///< extra data and two bytes of frame CRC at
                    ///< the end
  // uint16_t frameCRC;
} __packed FU_prot;


/**
 * @brief start of frame struct
 * 
 */
typedef struct {
  uint8_t FC;        ///< funtion code FU
  uint8_t frameLen;  ///< size in bytes from start of FU_prot address to last 
                     ///< byte of CRC
  uint8_t opcode;    ///< operation code, #FU_OPCODE_SOT
  uint8_t hash;      ///< mix of hardware and software version in this packet
  uint32_t fversion; ///< full version (device) or transfered firmware version
                     ///< (from host) [4b:hMajor, 4b:hMinor, 8b:fMajor,
                     ///< 16b:fMinor]
  uint16_t firmwareCRC; ///< BigEndian
  uint32_t fileSize;    ///< BigEndian
  uint16_t frameCRC;    ///< BigEndian
} __packed FU_SOT_prot;


// firmware upgrade protocol opcode
#define FU_OPCODE_SOT 2  ///< Start Of Transmission 
#define FU_OPCODE_DATA 3  ///< Data
#define FU_OPCODE_ACK 4  ///< Data or SOT ACK
#define FU_OPCODE_ABORT 5  ///< Error occurrence
#define FU_OPCODE_EOT 6  ///< End Of Transmission


#define FU_ERR_BAD_FRAME_CRC 1  ///< CRC in frame didn't match
#define FU_ERR_FLASH_WRITING 2  ///< error during writing to flash
#define FU_ERR_FLASH_ERASING 3  ///< error during erasing flash
#define FU_ERR_BAD_FRAME_LEN 4  ///< frame length didn't math
#define FU_ERR_BAD_FRAME_HASH 5  ///< incompatible frame hash
#define FU_ERR_BAD_OFFSET 6  ///< bad offset value in data frame
#define FU_ERR_BAD_OPCODE_SET 7  ///< bad FU opcode in frame
#define FU_ERR_FIR_NOT_ACCEPTED_YET 8  ///< firmware need to be accepted before 
  ///< firmware upgrade @see #FU_AcceptFirmware()
#define FU_ERR_BAD_FLASH_CRC 10  ///< 
#define FU_ERR_BAD_F_VERSION 11  ///< firmware version is for other partition
#define FU_ERR_BAD_H_VERSION 12  ///< hardware major version didn't match
#define FU_ERR_BAD_FILE_SIZE 13  ///< firmware size exceed partition capacity
  ///< or device has been reset during firmware upgrade
#define FU_ERR_BAD_PROT_VER 14  ///< incompatible firmware protocol version
#define FU_ERR_VERSION_IN_PACKAGE 15  ///< received data package with version 
  ///< part (should be EOT packet) 


/**
 * @brief FU protocol head size
 * 
 */
#define FU_PROT_HEAD_SIZE (1 + 1 + 1 + 1 + 2)

/**
 * @brief protocol version
 * 
 * maximal value is 16
 * 
 */
#define FU_PROT_VERSION 1


/**
 * @brief Initialize module
 * 
 * @param forceNoFirmwareCheck set true for sink when firmware verification
 *   should be forced to succeed
 */
void FU_Init(bool forceNoFirmwareCheck);


/**
 * @brief Mark current version as fully functional
 * 
 */
void FU_AcceptFirmware();


/**
 * 
 * @brief return base address of current working firmware
 * 
 * @return uint8_t* base address of current working firmware
 */
uint8_t *FU_GetCurrentFlashBase();


/**
 * @brief process new message as device
 * 
 * @param data message to process
 * @param info extra informations about message
 */
void FU_HandleAsDevice(const void *data, const prot_packet_info_t *info);

// === test module ===
//#define TEST_FU // comment to lock unit tests
#if TEST_ALL
#define TEST_FU 1
#endif
#if defined(TEST_FU)
/**
 * @brief Firmware upgrade testing procedure
 * 
 * @return uint8_t 
 */
uint8_t FU_Test();
#ifndef assert_param
void assert_param(uint8_t cond);
#endif
#endif

#endif /* FU_H_ */
