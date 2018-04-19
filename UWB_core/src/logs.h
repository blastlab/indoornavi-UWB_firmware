#ifndef _LOGS_H
#define _LOGS_H

#include <stdlib.h>
#include "platform/port.h" // define DBG

#define LOG_CRIT(...) LOG_Text('C', __VA_ARGS__)
#define LOG_ERR(...) LOG_Text('E', __VA_ARGS__)
#define LOG_WRN(...) LOG_Text('W', __VA_ARGS__)
#define LOG_INF(...) LOG_Text('I', __VA_ARGS__)
#define LOG_DBG(...) LOG_Text('D', __VA_ARGS__)
#define LOG_TEST(...) LOG_Text('T', __VA_ARGS__)

#define LOG_BASE64(BUF, LEN) log_bin(BUF, LEN)

// implmented in platform/log.c
int LOG_Text(char type, const char *frm, ...);

// implement in platform/log.c
int LOG_Bin(const void *bin, int size);

#endif // _LOGS_H
