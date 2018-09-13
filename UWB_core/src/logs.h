/**
 * @brief
 *
 * @file logs.h
 * @author Karol Trzcinski
 * @date 2018-06-29
 */
#ifndef _LOGS_H
#define _LOGS_H

#include <stdlib.h>

#define LOG_CRIT(...) LOG_Text('C', __VA_ARGS__)  ///< critical log
#define LOG_ERR(...) LOG_Text('E', __VA_ARGS__)  ///< log error
#define LOG_WRN(...) LOG_Text('W', __VA_ARGS__)  ///< log warning
#define LOG_INF(...) LOG_Text('I', __VA_ARGS__)  ///< log info
#define LOG_DBG(...) LOG_Text('D', __VA_ARGS__)  ///< log debug
#define LOG_TEST(...) LOG_Text('T', __VA_ARGS__)  ///< log text

#define LOG_BASE64(BUF, LEN) log_bin(BUF, LEN)  ///< log binary data (in base64)

/**
 * @brief log text data
 *
 * implemented in platform folder
 *
 * @param type log type
 * @param frm formating string
 * @param ... extra arguments
 * @return int
 */
int LOG_Text(char type, const char *frm, ...);

/**
 * @brief log binary data
 *
 * implemented in platform folder
 *
 * @param[in] bin pointer to binary data
 * @param[in] size of binary data
 * @return int
 */
int LOG_Bin(const void *bin, int size);

#endif // _LOGS_H
