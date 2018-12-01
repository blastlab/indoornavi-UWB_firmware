#include <stdio.h>
#include <string.h>

#include "logs.h"
#include "parsers/base64.h"
#include "platform/port.h"

#define LOG_BUF_LEN 1024
static char buf[LOG_BUF_LEN + 1];

void PORT_LogData(const void *p_bin, int p_size, const void *d_bin, int d_size, LOG_PacketCodes_t pc) {
	#if LOG_USB_EN
	if(pc == LOG_PC_Bin) {
		strcpy(buf, "B1000 ");
		int f = strlen(buf);
		if (BASE64_TextSize(d_size) + f >= LOG_BUF_LEN) {
			LOG_ERR(ERR_BASE64_TOO_LONG_OUTPUT, ((uint8_t *)d_bin)[0]);
		} else {
			f += BASE64_Encode((unsigned char*)(buf + f), d_bin, d_size);
			buf[f++] = '\n';
			CDC_Transmit_FS((uint8_t*)buf, f);
		}
	} else if(pc == LOG_PC_Txt) {
		CDC_Transmit_FS((uint8_t*)d_bin, d_size);
	}
	#endif
	#if LOG_SD_EN
	#endif
	LOG_BufPop();
}
