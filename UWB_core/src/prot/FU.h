/*
 * FU.h
 *
 *  Created on: 7 cze 2017
 *      Author: Karol Trzci�ski
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
extern char PROG_DESTINATION1; // defined in linker script
extern char PROG_DESTINATION2;
extern char PROG_START1; // isr vector location
extern char PROG_SETTINGS1;
#define FU_DESTINATION_1 ((void *)(&PROG_DESTINATION1))
#define FU_DESTINATION_2 ((void *)(&PROG_DESTINATION2))
#define FU_MAX_PROGRAM_SIZE ((int)(FU_DESTINATION_2 - FU_DESTINATION_1))

// place where current firmware version is stored (offsed from FLASH_BASE)
#define FU_VERSION_LOC (FLASH_PAGE_SIZE + 4)

// max tx data in target device is 4 bytes (+2 for CRC)
// but for source device it depends on data size in block
#define FU_MAX_DATA_SIZE (8 + 2)
//#define FU_MAX_DATA_SIZE				(64+2)

// size of one minimal piece of data
#define FU_BLOCK_SIZE 16

// raw firmware upgrade protocol struct
typedef struct {
  uint8_t FC;       // funtion code FU
  uint8_t frameLen; // size in bytes from start of FU_prot address to last byte
                    // of CRC
  uint8_t opcode;   // operation code[lobyte] and protocol version [hibyte]
  uint8_t hash;     // mix of hardware and software version in this packet
  uint16_t extra; // BigEndian, optional 16 bit data field with value depending
                  // from opcode
  uint8_t data[FU_MAX_DATA_SIZE];
  // uint16_t frameCRC; // two bytes of frame CRC at the end
} __packed FU_prot;

typedef struct {
  uint8_t FC;        // funtion code FU
  uint8_t frameLen;  // size in bytes from start of FU_prot address to last byte
                     // of CRC
  uint8_t opcode;    // operation code
  uint8_t hash;      // mix of hardware and software version in this packet
  uint32_t fversion; // full version (device) or transfered firmware version
                     // (from host) [4b:hMajor, 4b:hMinor, 8b:fMajor,
                     // 16b:fMinor]
  uint16_t firmwareCRC; // BigEndian
  uint32_t fileSize;    // BigEndian
  uint16_t frameCRC;    // BigEndian
} __packed FU_SOT_prot;
;

// firmware upgrade protocol opcode
#define FU_OPCODE_SOT 2
#define FU_OPCODE_DATA 3
#define FU_OPCODE_ACK 4
#define FU_OPCODE_ABORT 5
#define FU_OPCODE_EOT 6

// firmware upgrade protocol error code
#define FU_ERR_BAD_FRAME_CRC 1
#define FU_ERR_FLASH_WRITING 2
#define FU_ERR_FLASH_ERASING 3
#define FU_ERR_BAD_FRAME_LEN 4
#define FU_ERR_BAD_FRAME_HASH 5
#define FU_ERR_BAD_OFFSET 6
#define FU_ERR_BAD_OPCODE_SET 7
#define FU_ERR_FIR_NOT_ACCEPTED_YET 8
#define FU_ERR_BAD_FLASH_CRC 10
#define FU_ERR_BAD_F_VERSION 11
#define FU_ERR_BAD_H_VERSION 12
#define FU_ERR_BAD_FILE_SIZE 13
#define FU_ERR_BAD_PROT_VER 14
#define FU_ERR_VERSION_IN_PACKAGE 15

// ---Frame description ---
// At the begin SOT frame should be transmitted from Host to Device
// and Device should response ACK frame.
// Then is series of Data/ACK frames. ACK is required after each data frame
// On last data frame, Device response with EOT frame instead of ACK.
// It's extra information that firmware upgrade was fully successful.
//
// -- SOT -- Start Of Transmission -- Host->Device
// this frame should be sent at the beginning
// before frames with raw firmware data
// After receiving this frame, whole BANK for new firmware should be erased
// - Construction -
// [FC:1][frameLen:1][opcode:1][version:4][firmwareCRC:2][size:4][frameCRC:2]
// - frameLen 		- length of while frame, from address of frameLen
//									to the end of
//frame
// CRC,
// equal to 12.
// - opcode 			- set to FU_OPCODE_SOT
// - version			- [lowest::older] byte of new firmware version
// - firmwareCRC 	- CRC calculated only from complete raw firmware data
//									calculated in
//this
// same
// way as for each frame
// - size 				- big endian, size in bytes of complete
// raw
// firmware data
// - frameCRC			-	16 bit of CRC calculated on data from
// frameLen
// to
//									the end of offset,
//based
// on FU_CRC_POLY
//
//
// -- DEFAULT/Data frame -- Host->Device
// frame with firmware file data
// - Construction -
// [frameLen:1][opcode:1][version:1][offset:2][data:n][frameCRC:2]
// - frameLen 		- length of while frame, from address of frameLen
//									to the end of frame
//CRC
// - opcode 			- set to FU_OPCODE_DATA
// - version			- version of new firmware
// - offset				- offset in memory for transmitted data.
// 									Offset is expressed in
// FU_BLOCK_SIZE unit
// - data					- raw byte data, number of them can be
// readed
//									as
//frameLen-FU_PROT_HEAD_SIZE-2 (for CRC),
//									number of data should be
//multiple of FU_BLOCK_SIZE
// - frameCRC			-	16 bit of CRC calculated on data from
// frameLen
//									to the end of offset,
//based on FU_CRC_POLY
//
//
//
// -- ACK -- Device->Host
// Previously sent packet was correct and device ready to receive new frame
// - Construction -
// [frameLen:1][opcode:1][version:1][extra:2][frameCRC:2]
// - frameLen 		- length of while frame, from address of frameLen
//									to the end of frame CRC.
//Equal to 6
// - opcode 			- set to FU_OPCODE_ACK
// - version			- lowest byte of current firmware version
// - extra				- this field shouldn't be modified in
// ACK
// frame
//
//
//
// -- EOT -- Device->Host
// Successful firmware upgrade. Previously sent packet was correct and flashing
// success. Successful end of transmission process
// - Construction -
// [frameLen:1][opcode:1][version:1][extra:2][frameCRC:2]
// - frameLen 		- length of while frame, from address of frameLen
//									to the end of rame CRC.
//Equal to 6
// - opcode 			- set to FU_OPCODE_EOT
// - version			- lowest byte of current firmware version
// - extra				- this field shouldn't be modified in
//									EOT
//frame
//
//
//
// -- ABORT -- Device->Host
// Error info
// - Construction -
// [frameLen:1][opcode:1][version:1][errorCode:2][frameCRC:2]
// - frameLen 		- length of while frame, from address of frameLen
//									to the end of frame CRC.
//Equal to 8
// - opcode 			- set to FU_OPCODE_ABORT
// - version			- lowest byte of current firmware version
// - errorCode		- FU_ERR_code enumerate
//
//
//
// -- ASK_VER -- Host->Device
// ask about response ACK frame with current version of software
// - Construction -
// [frameLen:1][opcode:1][version:4][frameCRC:2]
// - frameLen 		- length of while frame, from address of frameLen
//									to the end of frame CRC.
//Equal to 6
// - opcode 			- set to FU_OPCODE_ASK_VER
// - version			- BigEndian, lowest byte of current firmware
// version
//
//
//

// extra define
#define FU_PROT_HEAD_SIZE (1 + 1 + 1 + 1 + 2)
#define FU_PROT_VERSION 1

// exported functions
void FU_Init(bool forceNoFirmwareCheck);
void FU_AcceptFirmware();
uint8_t *FU_GetCurrentFlashBase();
void FU_HandleAsDevice(const FU_prot *fup, const prot_packet_info_t *info);

// === test module ===
//#define TEST_FU // comment to lock unit tests
#if TEST_ALL
#define TEST_FU 1
#endif
#if defined(TEST_FU)
uint8_t FU_Test();
#ifndef assert_param
void assert_param(uint8_t cond);
#endif
#endif

#endif /* FU_H_ */
