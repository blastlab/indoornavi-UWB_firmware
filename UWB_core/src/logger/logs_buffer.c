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

#define CIRC_BUF_LEN 1024
#define LOG_BUF_LEN 1024

static char buf[LOG_BUF_LEN + 1];

struct {
	const int len;
	volatile int head;
	volatile int tail;
	volatile bool overflow;
	uint8_t data[CIRC_BUF_LEN];
} circBuf = {
		.len = CIRC_BUF_LEN,
		.head = 0,
		.tail = 0,
		.overflow = false
};

void LOG_BufClear() {
	circBuf.head = 0;
	circBuf.tail = circBuf.head;
}

static int BUF_GetHeadPacketLen() {
	// when buffer is empty
	if(circBuf.head == circBuf.tail) {
		return 0;
	}
	// the packet's length may not fit into left space of the buffer, so the conditions must be checked separately
	// when a header of the buffer fits into the end of memory block
	if(circBuf.head + FRAME_HEADER_SIZE < circBuf.len) {
		if(circBuf.len < circBuf.head + circBuf.data[circBuf.head + 1]) {				// when the packet does not fit into left space,
			circBuf.head = 0;																									// the header is also at the beginning,
		}																																		// else, start reading from current "head"
	}
	// when does not, the header is at the beginning of the buffer
	else {
		circBuf.head = 0;
	}
	return circBuf.data[circBuf.head + 1];
}

static inline LOG_PacketCodes_t BUF_GetHeadPacketOpcode() {
	return circBuf.data[circBuf.head];
}

static inline uint8_t * BUF_GetHeadPacketPtr() {
	return (uint8_t *)(&circBuf.data[circBuf.head]);
}

static void BUF_WriteHeader(LOG_Frame_s *frame) {
	// when the buffer is empty, the first byte is written to a current head == tail position
	circBuf.tail = (circBuf.tail == circBuf.head || circBuf.tail == 0) ? circBuf.tail : circBuf.tail + 1;
	memcpy(&circBuf.data[circBuf.tail], &frame->header, FRAME_HEADER_SIZE);
	circBuf.tail += FRAME_HEADER_SIZE - 1;
}

int BUF_WritePacket(uint8_t *data, LOG_PacketCodes_t packetCode, uint8_t len) {
	// when the buffer is full - { . . Tail Head . . .} || { Head . . . . Tail }
	if((((circBuf.tail + 1) % circBuf.len) == circBuf.head)) {
		goto buffer_full;
	}
	LOG_Frame_s frame = {
		.header.packetCode = (uint8_t)(packetCode),
		.header.len = (uint8_t)(len + FRAME_HEADER_SIZE + sizeof(frame.crc)),			// added header and crc length
		.data = data,
	};
	PORT_CrcReset();
	// feed CRC with opcode and packet length
	PORT_CrcFeed((uint8_t *)&frame, FRAME_HEADER_SIZE);
	frame.crc = PORT_CrcFeed(frame.data, len);				// feed CRC with data and write value to the packet's end

	// frame is ready to write to buffer here
	// when the Head is before the Tail - { . . . Head . . . Tail . HERE . . . .  }
	if(circBuf.head <= circBuf.tail) {
		// when the packet will fit into space at the end of the buffer
		if(circBuf.tail + frame.header.len < circBuf.len) {
			goto write_packet;
		}
		// else, when the packet will fit on the beginning of the buffer
		else if(frame.header.len <= circBuf.head) {
			// when the packet's header will fit into space at the end of the buffer, we will inform the reader about next frame
			// when no header fit into the end of buffer, the reader will know this and starts reading from head = 0
			if(circBuf.tail + FRAME_HEADER_SIZE < circBuf.len) {
				BUF_WriteHeader(&frame);
			}
			circBuf.tail = 0;
			goto write_packet;
		}
		// when the packet will not fit at the end and at the beginning of the buffer
		else {
			goto buffer_full;
		}
	}
	// else, when the Tail is before the Head and a packet will fit before actual Head - { . Tail . HERE . . . . Head . . . . . .  }
	else if(circBuf.tail < circBuf.head && circBuf.tail + frame.header.len < circBuf.head) {
		goto write_packet;
	}
	// else, when the buffer is full
	else {
		goto buffer_full;
	}

write_packet:
	BUF_WriteHeader(&frame);
	memcpy(&circBuf.data[++circBuf.tail], frame.data, len);
	circBuf.tail += len;
	circBuf.data[circBuf.tail] = (uint8_t)(frame.crc >> 8);
	circBuf.data[++circBuf.tail] = (uint8_t)(frame.crc & 0xFF);
	return frame.header.len;
buffer_full:
	circBuf.overflow = true;
	return 0;
}

void LOG_BufPop() {
	int packet_len = BUF_GetHeadPacketLen();
	// when the buffer is empty
	if(packet_len == 0) {
		return;
	}
CRITICAL(
	// set the Head at the end of a packet
	circBuf.head += packet_len - 1;
	// when another packet is ahead, set the Head at the beginning of it
	circBuf.head = (circBuf.head == circBuf.tail) ? circBuf.head : circBuf.head + 1;
	// when the Head exceeds the end of the buffer, set it to 0
	circBuf.head = (circBuf.len <= circBuf.head) ? 0 : circBuf.head;
)
}

int LOG_Text(char type, int num, const char* frm, va_list arg) {
  int n, f;
  // prefix np. "E101 "
  snprintf(buf, LOG_BUF_LEN, "%c%d ", type, num);
  f = strlen(buf);
  // zawartosc
  n = vsnprintf(buf + f, LOG_BUF_LEN - f, frm, arg) + f;
  if (n > 0 && n < LOG_BUF_LEN) {
    buf[n++] = '\r';
    buf[n++] = '\n';
    buf[n] = 0;
    CRITICAL(
    BUF_WritePacket((uint8_t*)buf, LOG_PC_Txt, n);
    )
  }
  return n;
}

int LOG_Bin(const void* bin, int size) {
  int n;
	CRITICAL(
	n = BUF_WritePacket((uint8_t*)bin, LOG_PC_Bin, size);
	)
  return n;
}

void LOG_Control(bool isSink) {
	// when buffer is empty
	if(BUF_GetHeadPacketLen() == 0) {
		return;
	}
	PORT_LogData(BUF_GetHeadPacketPtr(), BUF_GetHeadPacketLen(), BUF_GetHeadPacketOpcode(), isSink);
	if(circBuf.overflow) {
//		circBuf.overflow = false;
//		LOG_BufPop();
//		LOG_ERR(ERR_LOG_BUF_OVERFLOW);
	}
}
