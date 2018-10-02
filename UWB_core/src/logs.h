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

typedef enum {
  ERR_FLASH_ERASING,
  ERR_FLASH_WRITING,
  ERR_FLASH_OTHER,
  ERR_BAD_OPCODE,
  ERR_BAD_COMMAND,
  ERR_MAC_NO_MORE_BUFFERS,
  ERR_BAD_OPCODE_LEN,
  ERR_RF_BAD_CHANNEL,
  ERR_RF_BAD_BAUDRATE,
  ERR_RF_BAD_PREAMBLE_LEN,
  ERR_RF_BAD_PRF,
  ERR_RF_BAD_PAC,
  ERR_RF_BAD_CODE,
  ERR_RF_BAD_NSSFD,
  ERR_PARENT_FOR_SINK,
  ERR_PARENT_NEED_ANCHOR,
  ERR_MEASURE_FAILED_DID,
  ERR_MEASURE_FAILED_ANC_CNT,
  WRN_RANGING_TOO_SMALL_PERIOD,
  ERR_BLE_INACTIVE,
} ERR_codes;

typedef enum {
	WRN_FIRWARE_NOT_ACCEPTED_YET,
  WRN_SINK_ACCEPT_SINK,
  WRN_MAC_FRAME_BAD_OPCODE,
  WRN_MAC_UNSUPPORTED_MAC_FRAME,
  WRN_MAC_UNSUPPORTED_ACK_FRAME,
  WRN_MAC_TOO_BIG_FRAME,
  WRN_MAC_TX_ERROR,
  WRN_MAC_BAD_LEN_IN_MSG,
  WRN_MAC_NO_MORE_BUFFERS,
  WRN_CARRY_TARGET_NOBODY,
  WRN_CARRY_INCOMPATIBLE_VERSION,
} WRN_codes;

typedef enum {
  // network
  INF_DEVICE_TURN_ON = 100,
  INF_DEVICE_TURN_OFF,
  INF_BEACON,
  INF_DEV_ACCEPTED,
  INF_PARENT_DESCRIPTION,
  INF_PARENT_SET, 
  INF_PARENT_CNT,
  INF_SETTAGS_SET, 
  INF_SETANCHORS_SET, 
  INF_STATUS,
  INF_VERSION,
  INF_ROUTE,
  // radio
  INF_RF_SETTINGS = 200,
  INF_BLE_SETTINGS,
  // ranging
  INF_MEASURE_DATA = 300,
  INF_MEASURE_INFO,
  INF_MEASURE_CMD_CNT, 
  INF_MEASURE_CMD_SET,
  INF_RANGING_TIME,
  INF_TOA_SETTINGS,
  INF_CLEARED_PARENTS,
  INF_CLEARED_MEASURES,
  INF_CLEARED_PARENTS_AND_MEASURES,
  INF_CLEAR_HELP,
  // settings
  INF_SETTINGS_SAVED = 400,
  INF_SETTINGS_NO_CHANGES,
  // others
  INF_IMU_SETTINGS = 500,
  INF_FU_SUCCESS,
} INF_codes;

typedef enum {
	TEST_,
} TEST_codes;


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

void LOG_CRIT(ERR_codes code, ...);

void LOG_ERR(ERR_codes code, ...);

void LOG_WRN(WRN_codes code, ...);

void LOG_INF(INF_codes code, ...);

void LOG_DBG(const char *frm, ...);

void LOG_TEST(TEST_codes code, ...);
#endif // _LOGS_H
