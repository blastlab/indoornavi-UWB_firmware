#include <stdio.h>

int log_text(char type, const char *frm, ...)
{
#ifdef _CORTEX
    char buf[150];
    va_list arg;
    va_start(arg, frm);
    printf(, arg);
    int n = vsnprintf(buf, sizeof(buf), "%c " frm, arg);
    va_end(arg);
#else
    va_list arg;
    va_start(arg, frm);
    vrintf(frm, arg);
    va_end(arg);
#endif
}

int log_bin(char type, const void *bin, int size)
{
}
