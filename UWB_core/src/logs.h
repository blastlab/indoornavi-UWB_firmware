#ifndef _LOGS_H
#define _LOGS_H

#define LOG_CRIT(...) log_text('C', __VA_ARGS__)
#define LOG_ERR(...) log_text('E', __VA_ARGS__)
#define LOG_INF(...) log_text('I', __VA_ARGS__)
#define LOG_DBG(...) log_text('D', __VA_ARGS__)
#define LOG_TEST(...) log_text('T', __VA_ARGS__)

#define LOG_BASE64(BUF, LEN) log_bin('B', BUF, LEN)

int log_text(char type, const char *frm, ...);

int log_bin(char type, const void *bin, int size);

#endif // _LOGS_H