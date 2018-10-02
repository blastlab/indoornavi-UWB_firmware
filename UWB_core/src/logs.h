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
#include <stdarg.h>

#undef ADD_ITEM
#undef ADD_ITEM_M
#define ADD_ITEM(CODE,ENUM_VALUE,MESSAGE) ENUM_VALUE,
#define ADD_ITEM_M(CODE,ENUM_VALUE,MESSAGE) ENUM_VALUE,

typedef enum {
#include "logger/logs_crit.h"
	CRIT_codes_N
} CRIT_codes;

typedef enum {
#include "logger/logs_err.h"
	ERR_codes_N
} ERR_codes;

typedef enum {
#include "logger/logs_wrn.h"
	WRN_codes_N
} WRN_codes;

typedef enum {
#include "logger/logs_inf.h"
	INF_codes_N
} INF_codes;

typedef enum {
#include "logger/logs_test.h"
	TEST_codes_N
} TEST_codes;

#undef ADD_ITEM
#undef ADD_ITEM_M

/**
 * @brief logger informations code tester
 * 
 */
void LOG_SelfTest();

/**
 * @brief log text data
 *
 * implemented in platform folder
 *
 * @param type log type
 * @param type log identification code
 * @param frm formating string
 * @param ... extra arguments
 * @return int
 */
int LOG_Text(char type, int num, const char *frm, va_list arg);

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

/**
 * @brief log criticcal information
 * 
 * @param code of message
 * @param ... extra arguments
 */
void LOG_CRIT(ERR_codes code, ...);

/**
 * @brief log error information
 * 
 * @param code of message
 * @param ... extra arguments
 */
void LOG_ERR(ERR_codes code, ...);

/**
 * @brief log warning information
 * 
 * @param code of message
 * @param ... extra arguments
 */
void LOG_WRN(WRN_codes code, ...);

/**
 * @brief log informations
 * 
 * @param code of message
 * @param ... extra arguments
 */
void LOG_INF(INF_codes code, ...);

/**
 * @brief debug information logger
 * 
 * @param code of message
 * @param ... extra arguments
 */
void LOG_DBG(const char *frm, ...);

/**
 * @brief target device integration test logger
 * 
 * @param code of message
 * @param ... extra arguments
 */
void LOG_TEST(TEST_codes code, ...);
#endif // _LOGS_H
