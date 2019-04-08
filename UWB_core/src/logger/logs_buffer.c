/*
 * logs_buffer.c
 *
 *  Created on: 1 gru 2018
 *      Author: DawidPeplinski
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "logs.h"
#include "parsers/base64.h"
#include "platform/port.h"
#include "logs_buffer.h"

static uint16_t CrcFeed(uint16_t * crc, const void* bytes, int nBytes) {
	*crc = PORT_CrcFeed(crc, bytes, nBytes);
  return *crc;
}

static inline void BUF_FitPacketToBuf(circular_buff_s * circBuf) {
	// the packet's length may not fit into left space of the buffer, so the conditions must be checked separately
	// when a header of the packet fits into the end of memory block
	if(circBuf->head + FRAME_HEADER_SIZE <= CIRC_BUF_LEN) {
		// when the packet does fit into left space
		if(circBuf->head + circBuf->data[circBuf->head + 1] <= CIRC_BUF_LEN) {
			// the head's location is correct, overrun
		}
		// when does not, the header is repeated at the beginning of the buffer
		else {
			circBuf->head = 0;
		}
	}
	// when does not, the header is repeated at the beginning of the buffer
	else {
		circBuf->head = 0;
	}
}

static void BUF_WriteHeader(circular_buff_s * circBuf, LOG_Frame_s *frame) {
	// when the buffer is empty, writing first byte to actual head
	circBuf->tail = (circBuf->tail == (circBuf->head + 1)) ? circBuf->tail - 1 : circBuf->tail;
	memcpy(&circBuf->data[circBuf->tail], &frame->header, FRAME_HEADER_SIZE);
	circBuf->tail += FRAME_HEADER_SIZE;
}

void BUF_Clear(circular_buff_s * circBuf) {
	circBuf->head = 0;
	circBuf->tail = 1;
	circBuf->overflow = false;
}

LOG_PacketCodes_t BUF_GetHeadPacketOpcode(circular_buff_s * circBuf) {
	BUF_FitPacketToBuf(circBuf);
	return circBuf->data[circBuf->head];
}

int BUF_GetHeadPacketLen(circular_buff_s * circBuf) {
	// when buffer is empty
	if((circBuf->head + 1) == circBuf->tail) {
		return 0;
	} else {
		BUF_FitPacketToBuf(circBuf);
		return circBuf->data[circBuf->head + 1];
	}
}

int BUF_GetHeadPacketTextLen(circular_buff_s * circBuf) {
	// when buffer is empty
	if((circBuf->head + 1) == circBuf->tail) {
		return 0;
	} else {
		BUF_FitPacketToBuf(circBuf);
		return circBuf->data[circBuf->head + 1] - FRAME_HEADER_SIZE - 2;
	}
}

uint8_t * BUF_GetHeadPacketPtr(circular_buff_s * circBuf) {
	BUF_FitPacketToBuf(circBuf);
	return (uint8_t *)(&circBuf->data[circBuf->head]);
}

uint8_t * BUF_GetHeadPacketTextPtr(circular_buff_s * circBuf) {
	BUF_FitPacketToBuf(circBuf);
	return (uint8_t *)(&circBuf->data[circBuf->head + FRAME_HEADER_SIZE]);
}

int BUF_WritePacket(circular_buff_s * circBuf, uint8_t *data, LOG_PacketCodes_t packetCode, uint8_t len) {
	// when the buffer is full - { . . Tail == Head . . .}
	if(circBuf->tail == circBuf->head) {
		goto buffer_full;
	}
	LOG_Frame_s frame;
	// when packet code is 0, the data is already a packet and it is needed only to be written into the buffer
	if(packetCode == 0) {
		frame.header.packetCode = (uint8_t)data[0];
		frame.header.len = (uint8_t)data[1];
		frame.data = &data[2];
		frame.crc = (uint16_t)(data[data[1] - 2] << 8 | data[data[1] - 1]);
		len = frame.header.len - FRAME_HEADER_SIZE - sizeof(frame.crc);
	} else {
		frame.header.packetCode = (uint8_t)(packetCode);
		frame.header.len = (uint8_t)(len + FRAME_HEADER_SIZE + sizeof(frame.crc));
		frame.data = data;
		frame.crc = 0xFFFF;
		// feed CRC with opcode and packet length
		CrcFeed(&frame.crc, (uint8_t *)&frame, FRAME_HEADER_SIZE);
		// feed CRC with data and write value to the packet's end
		CrcFeed(&frame.crc, frame.data, len);
	}
	// frame is ready to write to buffer
	// when the Head is before the Tail - { . . . Head . . . Tail HERE . . . .  }
	if(circBuf->head < circBuf->tail) {
		// when the buffer is empty, first byte will be written to actual head
		if(circBuf->tail == (circBuf->head + 1)) {
			circBuf->tail--;
		}
		// when the packet will fit into space at the end of the buffer
		if(circBuf->tail + frame.header.len <= CIRC_BUF_LEN) {
			goto write_packet;
		}
		// else, when the packet will fit on the beginning of the buffer
		else if(frame.header.len <= circBuf->head) {
			// when the packet's header will fit into space at the end of the buffer, we will inform the reader about next frame
			// when no header fit into the end of buffer, the reader will know this and starts reading from head = 0
			if(circBuf->tail + FRAME_HEADER_SIZE <= CIRC_BUF_LEN) {
				BUF_WriteHeader(circBuf, &frame);
			}
			circBuf->tail = 0;
			goto write_packet;
		}
		// when the packet will not fit at the end and at the beginning of the buffer
		else {
			goto buffer_full;
		}
	}
	// else, when the Tail is before the Head and a packet will fit before actual Head - { . Tail . HERE . . . . Head . . . . . .  }
	else if(circBuf->tail < circBuf->head && circBuf->tail + frame.header.len <= circBuf->head) {
		goto write_packet;
	}
	// else, when the buffer is full
	else {
		goto buffer_full;
	}

write_packet:
	BUF_WriteHeader(circBuf, &frame);
	memcpy(&circBuf->data[circBuf->tail], frame.data, len);
	circBuf->tail += len;
	circBuf->data[circBuf->tail] = (uint8_t)(frame.crc >> 8);
	circBuf->data[++circBuf->tail] = (uint8_t)(frame.crc & 0xFF);
	circBuf->tail++;
	return frame.header.len;
buffer_full:
	circBuf->overflow = true;
	return 0;
}

int BUF_IsEmpty(circular_buff_s * circBuf) {
	if((circBuf->head + 1) == circBuf->tail)
		return 1;
	else
		return 0;
}

void BUF_Pop(circular_buff_s * circBuf) {
	// move the Head to the beginning of a next packet or by zero when buffer is empty
	circBuf->head += BUF_GetHeadPacketLen(circBuf);
	// when there is no more packets in buffer
	circBuf->head = (circBuf->head == circBuf->tail) ? circBuf->head - 1 : circBuf->head;
	// when the Head exceeds the end of the buffer, set it to 0
	circBuf->head = (circBuf->head < CIRC_BUF_LEN) ? circBuf->head : 0;
	// when something in the buffer was corrupted, clear it
	if(LOG_PC_CodesCount <= circBuf->data[circBuf->head]) {
		BUF_Clear(circBuf);
	}
}
