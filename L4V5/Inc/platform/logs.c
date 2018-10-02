#include <stdarg.h>
#include <stdio.h>
#include "parsers/base64.h"
#include "platform/port.h"

#define LOG_USB_EN 1
#define LOG_SD_EN 0
#define LOG_USB_UART 0

#define LOG_BUF_LEN 256

static char buf[LOG_BUF_LEN + 1];

int LOG_Text(char type, int num, const char* frm, va_list arg) {
	int n, f;

	// prefix np. "E101 "
	f = 0;
	buf[f++] = type;
	itoa(num, &buf[f], 10);
	f = strlen(buf);
	buf[f++] = ' ';

	// zawartosc
	n = vsnprintf(buf + f, LOG_BUF_LEN - f, frm, arg) + f;

	if (n > 0 && n < LOG_BUF_LEN) {
		buf[n++] = '\r';
		buf[n++] = '\n';
		buf[n] = 0;
// wyslij dane na SD i do USB
#if LOG_USB_EN
		CDC_Transmit_FS((uint8_t*)buf, n);
#endif
#if LOG_LCD_EN
		if (type == LOG_ERR)
		lcd_err(buf);
#endif
#if LOG_SD_EN
#endif
	}
	return n;
}

int LOG_Bin(const void* bin, int size) {
	int f;
	f = 0;
	buf[f++] = 'B';
	buf[f++] = ' ';
	if (BASE64_TextSize(size) + f >= LOG_BUF_LEN) {
		LOG_Text('E', "logbin: too big binary file! FC:%xh", ((uint8_t*)bin)[0]);
		return 0;
	} else {
		f += BASE64_Encode((unsigned char*)(buf + f), bin, size);
		buf[f++] = '\n';
		CDC_Transmit_FS((uint8_t*)buf, f);
		return f;
	}
}
