/*
 * logs_buffer.h
 *
 *  Created on: 4 kwi 2019
 *      Author: Administrator
 */

#ifndef MIDDLEWARES_UWB_LOGGER_LOGS_BUFFER_H_
#define MIDDLEWARES_UWB_LOGGER_LOGS_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>

#define CIRC_BUF_LEN 1024

typedef enum {
	LOG_PC_Bin = 0x01,
	LOG_PC_Txt = 0x02,
	LOG_PC_Ack = 0x03,
	LOG_PC_Nack = 0x04,
	LOG_PC_CodesCount = 0x05
} LOG_PacketCodes_t;

typedef struct {
	uint8_t packetCode;
	uint8_t len;
} LOG_FrameHeader_s;
#define FRAME_HEADER_SIZE sizeof(LOG_FrameHeader_s)

typedef struct {
	LOG_FrameHeader_s header;
	uint8_t *data;
	uint16_t crc;
} LOG_Frame_s;

typedef struct {
	volatile int head;
	volatile int tail;
	volatile bool overflow;
	uint8_t data[CIRC_BUF_LEN];
} circular_buff_s;

void BUF_Clear(circular_buff_s * circBuf);
LOG_PacketCodes_t BUF_GetHeadPacketOpcode(circular_buff_s * circBuf);
int BUF_GetHeadPacketLen(circular_buff_s * circBuf);
int BUF_GetHeadPacketTextLen(circular_buff_s * circBuf);
uint8_t * BUF_GetHeadPacketPtr(circular_buff_s * circBuf);
uint8_t * BUF_GetHeadPacketTextPtr(circular_buff_s * circBuf);
int BUF_WritePacket(circular_buff_s * circBuf, uint8_t *data, LOG_PacketCodes_t packetCode, uint8_t len);
int BUF_IsEmpty(circular_buff_s * circBuf);
void BUF_Pop(circular_buff_s * circBuf);

// Buf clear, write and pop must be called in a critical section!

#endif /* MIDDLEWARES_UWB_LOGGER_LOGS_BUFFER_H_ */
