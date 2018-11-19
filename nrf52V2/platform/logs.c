#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "logs.h"
#include "parsers/base64.h"
#include "port.h"

#define LOG_BUF_LEN 1024
#define HEADER_SIZE 2

static char buf[LOG_BUF_LEN + 1];
uint8_t PORT_UsbUartTransmit(uint8_t* buf, uint16_t len);

typedef enum {				// the four of more significant bits
	LOG_PC_Bin = 0x10,
	LOG_PC_Txt = 0x20,
	LOG_PC_Ack = 0x30,
	LOG_PC_Other = 0x40,
} LOG_PacketCode_t;

typedef enum {				// the four of less significant bits
	LOG_MC_Inf = 0x01,
	LOG_MC_Wrn = 0x02,
	LOG_MC_Err = 0x03,
	LOG_MC_Meas = 0x04,
} LOG_MessageCode_t;

struct {
	const int len;
	int head;
	int tail;
	uint8_t data[LOG_BUF_LEN];
} circBuf = {
		.len = LOG_BUF_LEN,
		.head = 0,
		.tail = 0
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
	if(circBuf.head + HEADER_SIZE < circBuf.len) {
		if(circBuf.len < circBuf.head + circBuf.data[circBuf.head + 1]) {		// when the packet does not fit into left space,
			circBuf.head = 0;																									// the header is also at the beginning,
		}																																		// else, start reading from current "head"
	}
	// when does not, the header is at the beginning of the buffer
	else {
		circBuf.head = 0;
	}
	return circBuf.data[circBuf.head + 1];
}

// Add CRITICAL SECTION here!
void BUF_WritePacket(uint8_t *data, LOG_PacketCode_t packetCode, LOG_MessageCode_t messageCode, uint8_t len) {
	// when the buffer is full - { . . Tail Head . . .} || { Head . . . . Tail }
	if(((circBuf.tail + 1) == circBuf.head) || ((circBuf.head == 0) && (circBuf.tail == (circBuf.len - 1)))) {
		goto buffer_full;
	}
	struct {
		uint8_t opcode;
		uint8_t len;
		uint8_t *data;
		uint16_t CRC;

	} frame = {
		.opcode = (uint8_t)(packetCode | messageCode),
		.len = (uint8_t)(len + HEADER_SIZE + 2),			// added header and crc length
		.data = data,
	};
	// when opcode is TXT, encode data to BASE64 before CRC (or do not because it is done when Usb transmit comes)
	PORT_CrcReset();
	// feed CRC with opcode and packet length
	PORT_CrcFeed((uint8_t *)&frame, HEADER_SIZE);
	frame.CRC = PORT_CrcFeed(frame.data, len);				// feed CRC with data and write value to the packet's end

	// frame is ready to write to buffer here
	// when the Head is before the Tail - { . . . Head . . . Tail . HERE . . . .  }
	if(circBuf.head <= circBuf.tail) {
		// when the packet will fit into space at the end of the buffer
		if(circBuf.tail + frame.len < circBuf.len) {
			goto write_packet;
		}
		// else, when the packet will fit on the beginning of the buffer
		else if(frame.len <= circBuf.head) {
			// when the packet's header will fit into space at the end of the buffer, we will inform the reader about next frame
			// when no header fit into the end of buffer, the reader will know this and starts reading from head = 0
			if(circBuf.tail + HEADER_SIZE < circBuf.len) {
				circBuf.tail = (circBuf.tail == circBuf.head || circBuf.tail == 0) ? circBuf.tail : circBuf.tail + 1;
				circBuf.data[circBuf.tail] = frame.opcode;
				circBuf.data[++circBuf.tail] = frame.len;
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
	else if(circBuf.tail < circBuf.head && circBuf.tail + frame.len < circBuf.head) {
		goto write_packet;
	}
	// else, when the buffer is full
	else {
		goto buffer_full;
	}

write_packet:
	// when the buffer is empty, the first byte is written to a current head == tail position
	circBuf.tail = (circBuf.tail == circBuf.head || circBuf.tail == 0) ? circBuf.tail : circBuf.tail + 1;
	circBuf.data[circBuf.tail] = frame.opcode;
	circBuf.data[++circBuf.tail] = frame.len;
	memcpy(&circBuf.data[++circBuf.tail], frame.data, len);
	circBuf.tail += len;
	circBuf.data[circBuf.tail] = (uint8_t)(frame.CRC >> 8);
	circBuf.data[++circBuf.tail] = (uint8_t)(frame.CRC & 0xFF);
	return;
buffer_full:
//	TODO handle buffer overflow!
//	BUF_Pop();
//	LOG_CRIT();
	return;
}

void BUF_Pop() {
	int packet_len = BUF_GetHeadPacketLen();
	// when the buffer is empty
	if(packet_len == 0) {
		return;
	}
	// set the Head at the end of a packet
	circBuf.head += packet_len - 1;
	// when another packet is ahead, set the Head at the beginning of it
	circBuf.head = (circBuf.head == circBuf.tail) ? circBuf.head : circBuf.head + 1;
	// when the Head exceeds the end of the buffer, set it to 0
	circBuf.head = (circBuf.len <= circBuf.head) ? 0 : circBuf.head;
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
    BUF_WritePacket((uint8_t*)buf, LOG_PC_Txt, LOG_MC_Inf, n);
    )
  }
  return n;
}

int LOG_Bin(const void* bin, int size) {
  int f;
  f = 0;
  buf[f++] = 'B';
  buf[f++] = ' ';
  if (BASE64_TextSize(size) + f >= LOG_BUF_LEN) {
    LOG_ERR(ERR_BASE64_TOO_LONG_OUTPUT, ((uint8_t*)bin)[0]);
    return 0;
  } else {
    f += BASE64_Encode((unsigned char*)(buf + f), bin, size);
    buf[f++] = '\n';
    PORT_UsbUartTransmit((uint8_t*)buf, f);
    return f;
  }
}

void LOG_Control() {
	int packet_len = BUF_GetHeadPacketLen();
	// when buffer is empty
	if(packet_len == 0) {
		return;
	}
	// wyslij dane na SD i do USB
	#if LOG_USB_EN
			PORT_UsbUartTransmit((uint8_t *)(&circBuf.data[circBuf.head] + HEADER_SIZE), packet_len - HEADER_SIZE - 2);
	#endif
	#if LOG_LCD_EN
			if (type == LOG_ERR)
				lcd_err(buf);
	#endif
	#if LOG_SD_EN
	#endif
	BUF_Pop();
}
