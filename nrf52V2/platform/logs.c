#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "logs.h"
#include "parsers/base64.h"
#include "port.h"

#define LOG_BUF_LEN 256

static char buf[LOG_BUF_LEN + 1];
uint8_t PORT_UsbUartTransmit(uint8_t* buf, uint16_t len);

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
// wyslij dane na SD i do USB
#if LOG_USB_EN
    PORT_UsbUartTransmit((uint8_t*)buf, n);
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
    LOG_ERR(ERR_BASE64_TOO_LONG_OUTPUT, ((uint8_t*)bin)[0]);
    return 0;
  } else {
    f += BASE64_Encode((unsigned char*)(buf + f), bin, size);
    buf[f++] = '\n';
    PORT_UsbUartTransmit((uint8_t*)buf, f);
    return f;
  }
}
