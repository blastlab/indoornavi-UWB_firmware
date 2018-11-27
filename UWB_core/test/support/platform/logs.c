#include <stdarg.h>
#include <stdio.h>

int LOG_Text(char type, const char *frm, ...) {
  va_list arg;
  va_start(arg, frm);
  vprintf(frm, arg);
  va_end(arg);
}