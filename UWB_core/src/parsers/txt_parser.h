/**
 * @brief text parser engine
 *
 * Each message should be transformated from text to correct struct and function
 * code. Struct with function code should be transformed to binary message and
 * parsed by binary parser. Such mechanism helps to keep code consistent and
 * help to avoid code doubling in text and binary parser.
 *
 * @file txt_parser.h
 * @author Karol Trzcinski
 * @date 2018-06-28
 */
#ifndef _TXT_PARSER_H
#define _TXT_PARSER_H

#include <ctype.h>  // tolower

#include "../mac/carry.h"
#include "base64.h"
#include "bin_const.h"
#include "bin_struct.h"
#include "printer.h"
#include "ranging.h"
#include "tools.h"

typedef const char cchar;

/**
 * @brief text parser engine circular buffer structure
 *
 */
typedef struct {
  cchar* cmd;          ///< pointer to start of current message to parse
  cchar* const start;  ///< pointer to start of buffer
  cchar* const end;    ///< pointer to start of buffer + buffer length
  int cnt;             ///< numer of messages ready to parse
} txt_buf_t;

/**
 * @brief text parser engine callback definition
 *
 */
typedef void (*txt_parser_cb)(const txt_buf_t* buf,
                              const prot_packet_info_t* info);

/**
 * @brief text parser engine callback struct
 *
 */
typedef struct {
  const char* cmd;         ///< command string
  const txt_parser_cb cb;  ///< calback function routine
} txt_cb_t;

/**
 * @brief
 *
 * @param[in] buf buffer data, especially buf->end and buf->start
 * @param[in] cmd pointer to place where start searching
 * @param[in] num number of parameter to point
 * @return cchar* pointer to parameter number num or 0
 */
cchar* TXT_PointParamNumber(const txt_buf_t* buf, cchar* cmd, int num);

/**
 * @brief convert text number to int from circular buffer
 *
 * @param[in] buf buffer data, especially buf->end and buf->start
 * @param[in] ptr pointer to number in text
 * @param[in] base number base, eg. 10 or 16
 * @return int number value or 0
 */
int TXT_AtoI(const txt_buf_t* buf, cchar* ptr, int base);

/**
 * @brief get parametr number value, form "cmd:value" from circular buffer
 *
 * @param[in] buf buffer to search in
 * @param[in] cmd parameter name
 * @param[in] base number base, eg. 10 or 16
 * @return int parameter value or -1
 */
int TXT_GetParam(const txt_buf_t* buf, cchar* cmd, int base);

/**
 * @brief get parametr number value, form "cmd:value" from circular buffer
 *
 * @param[in] buf buffer to search in
 * @param[in] num number of param to get, at 0 is cmd
 * @param[in] base number base, eg. 10 or 16
 * @return int parameter value or -1
 */
int TXT_GetParamNum(const txt_buf_t* buf, int num, int base);

/**
 * @brief check if command starts with in circular buffer
 *
 * @param[in] buf buffer to search in
 * @param[in] cmd command start message
 * @return true buf->cmd starts with cmd
 * @return false buf->cmd doesn't starts with cmd
 */
bool TXT_StartsWith(const txt_buf_t* buf, cchar* cmd);

/**
 * @brief take input to text parser circular buffer
 *
 * Ignore \\r and split by \\n
 *
 * @param[in] str pointer to new input string
 * @param[in] len length of new input string
 */
void TXT_Input(const char* str, int len);

/**
 * @brief look for any new text message to parse
 *
 * This function should be called frequently from main program loop
 * to start parsing new text message.
 *
 */
void TXT_Control();
#endif  // _TXT_PARSER_H
