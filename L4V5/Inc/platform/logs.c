#include "base64.h"
#include "port.h"
#include <stdarg.h>
#include <stdio.h>


#define LOG_BUF_LEN 256

static char buf[LOG_BUF_LEN + 1];

int log_text(char type, const char *frm, ...) {
  int n, f;
  va_list arg;
  va_start(arg, frm);
  // itoa(type, buf, 10);
  // f = strlen(buf);
  f = 0;
  buf[f++] = type;
  buf[f++] = ' ';
  n = vsnprintf(buf + f, LOG_BUF_LEN - f, frm, arg) + f;
  va_end(arg);

  if (n > 0 && n < LOG_BUF_LEN) {
    buf[n++] = '\r';
    buf[n++] = '\n';
    buf[n] = 0;
// wyslij dane na SD i do USB
#if LOG_USB_EN
    CDC_Transmit_FS((uint8_t *)buf, n);
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

int log_bin(char type, const void *bin, int size) {
  int f;
  f = 0;
  buf[f++] = type;
  buf[f++] = ' ';
  if (base64_textSize(size) + f >= LOG_BUF_LEN) {
    log_text('E', "logbin: too big binary file! FC:%xh", ((uint8_t *)bin)[0]);
    return 0;
  } else {
    f += base64_encode((unsigned char *)(buf + f), bin, size);
    buf[f++] = '\n';
    CDC_Transmit_FS((uint8_t *)buf, f);
    return f;
  }
}
