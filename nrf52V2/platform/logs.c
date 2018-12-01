#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "mac.h"
#include "logs.h"
#include "parsers/base64.h"
#include "port.h"

#define LOG_BUF_LEN 1024
static char buf[LOG_BUF_LEN + 1];
uint8_t PORT_UsbUartTransmit(uint8_t *buf, uint16_t len);

void PORT_LogData(const void *p_bin, int p_size, const void *d_bin, int d_size, LOG_PacketCodes_t pc) {
	// wyslij dane na SD, po SPI i do USB
	#if LOG_SPI_EN && ETH_SPI_SS_PIN
	if(settings.mac.role == RTLS_SINK) {
		PORT_SpiTx((uint8_t *)p_bin, p_size, ETH_SPI_SS_PIN);
	}
	#endif
	#if LOG_USB_EN
	if(pc == LOG_PC_Bin) {
		strcpy(buf, "B1000 ");
		int f = strlen(buf);
		if (BASE64_TextSize(d_size) + f >= LOG_BUF_LEN) {
			LOG_ERR(ERR_BASE64_TOO_LONG_OUTPUT, ((uint8_t *)d_bin)[0]);
		} else {
			f += BASE64_Encode((unsigned char*)(buf + f), d_bin, d_size);
			buf[f++] = '\n';
			PORT_UsbUartTransmit((uint8_t*)buf, f);
		}
	} else if(pc == LOG_PC_Txt) {
		PORT_UsbUartTransmit((uint8_t*)d_bin, d_size);
	}
	#endif
	#if LOG_SD_EN
	#endif
	LOG_BufPop();
}
