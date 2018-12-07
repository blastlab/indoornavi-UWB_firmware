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

/**
 * \brief This is a trace enums, useful to track application behavior
 */
typedef enum {
	TRACE_EMPTY = 0,
	TRACE_SYSTICK = 1,
	TRACE_PREPARE_SLEEP,
	TRACE_GO_SLEEP,
	TRACE_WAKEUP,
	TRACE_DW_IRQ_ENTER,
	TRACE_DW_IRQ_RX,
	TRACE_DW_IRQ_TX,
	TRACE_DW_IRQ_TO,
	TRACE_DW_IRQ_ERR,
	TRACE_DW_IRQ_EXIT,
	TRACE_SLOT_TIM_ENTER,
	TRACE_SLOT_TIM_EXIT,
	TRACE_WAKE_TIM_ENTER,
	TRACE_WAKE_TIM_EXIT,
	TRACE_IMU_IRQ_ENTER,
	TRACE_IMU_IRQ_EXIT,
	TRACE_USART_IRQ_ENTER,
	TRACE_USART_IRQ_EXIT,
} TRACE_t;

/**
 * @brief LOG some event in history for debug purpose
 *
 */
void LOG_Trace(TRACE_t);

/**
 * @brief sending messages from circled buffer for logs
 *
 */
void LOG_Control();

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
