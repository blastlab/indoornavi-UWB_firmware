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
#include <stdint.h>
#include <stdbool.h>

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

typedef struct {
		uint8_t packetCode;
		uint8_t len;
} LOG_FrameHeader_s;
#define FRAME_HEADER_SIZE sizeof(LOG_FrameHeader_s)

typedef struct {
		LOG_FrameHeader_s header;
		uint8_t *data;
		uint16_t crc;
}	LOG_Frame_s;

/**
 * @brief log data from logger's buffer
 *
 * implemented in platform folder
 * call LOG_BufPop() after successful transaction
 *
 * @param[in] bin pointer to raw binary packet's data
 * @param[in] size of binary raw packet's data
 * @param[in] isSink specifies if the device's role equals SINK
 */
void PORT_LogData(const void *bin, int size, LOG_PacketCodes_t pc, bool isSink);

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
 * @brief pop a message from logger's buffer
 *
 */
void LOG_BufPop();

/**
 * @brief pull messages from logger's buffer
 * @param[in] isSink specifies if the device's role equals SINK
 */
void LOG_Control(bool isSink);

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
