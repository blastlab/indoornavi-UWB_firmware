#include <stdio.h>
#include <string.h>

#include "logger/logs.h"
#include "parsers/base64.h"
#include "platform/port.h"

#define LOG_BUF_LEN 1024
static char buf[LOG_BUF_LEN + 1];

void PORT_LogData(const void *bin, int size, LOG_PacketCodes_t pc, bool isSink) {
	#if LOG_USB_EN
	if(pc == LOG_PC_Bin) {
		strcpy(buf, "B1000 ");
		int f = strlen(buf);
		if (BASE64_TextSize(size - FRAME_HEADER_SIZE - 2) + f >= LOG_BUF_LEN) {
			LOG_ERR(ERR_BASE64_TOO_LONG_OUTPUT, ((uint8_t *)bin)[FRAME_HEADER_SIZE]);
		} else {
			f += BASE64_Encode((unsigned char*)(buf + f), ((uint8_t *)bin + FRAME_HEADER_SIZE), size - FRAME_HEADER_SIZE - 2);
			buf[f++] = '\n';
			CDC_Transmit_FS((uint8_t*)buf, f);
		}
	} else if(pc == LOG_PC_Txt) {
		CDC_Transmit_FS(((uint8_t *)bin + FRAME_HEADER_SIZE), size - FRAME_HEADER_SIZE - 2);
	}
	#endif
	#if LOG_SD_EN
	#endif
	LOG_BufPop();
}
