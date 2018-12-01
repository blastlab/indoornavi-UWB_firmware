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
#undef COMMENT
#undef ARG

#define ADD_ITEM(CODE,ENUM_VALUE,MESSAGE) ENUM_VALUE,
#define ADD_ITEM_M(CODE,ENUM_VALUE,MESSAGE) ENUM_VALUE,
#define COMMENT(X)
#define ARG(NAME,DESCRIPTION)

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
#undef COMMENT
#undef ARG

typedef enum {
	LOG_PC_Bin = 0x01,
	LOG_PC_Txt = 0x02,
	LOG_PC_Ack = 0x03,
	LOG_PC_Nack = 0x04
} LOG_PacketCodes_t;

/**
 * @brief log data from logger's buffer
 *
 * implemented in platform folder
 * call LOG_BufPop() after successful transaction
 *
 * @param[in] p_bin pointer to raw binary packet's data
 * @param[in] p_size of binary raw packet's data
 * @param[in] d_bin pointer to binary/text data within a packet
 * @param[in] d_size of binary/text data within a packet
 * @param[in] pc packet code
 */
void PORT_LogData(const void *p_bin, int p_size, const void *d_bin, int d_size, LOG_PacketCodes_t pc);

/**
 * @brief pop a message from logger's buffer
 *
 */
void LOG_BufPop();

/**
 * @brief pull messages from logger's buffer
 *
 */
void LOG_Control();

/**
 * @brief logger informations code tester
 *
 */
void LOG_SelfTest();

/**
 * @brief write text data to the logger's buffer
 *
 * @param type log type
 * @param type log identification code
 * @param frm formating string
 * @param ... extra arguments
 * @return int
 */
int LOG_Text(char type, int num, const char *frm, va_list arg);

/**
 * @brief write binary data to the logger's buffer
 *
 * @param[in] bin pointer to binary data
 * @param[in] size of binary data
 * @return int
 */
int LOG_Bin(const void *bin, int size);

/**
 * @brief log critical information
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
