/**
 * @brief binary data parser engine
 *
 * core of this engine is in bin_parser_cb where each
 * callback is implemented and added to the callbacks table.
 *
 * @file bin_parser.h
 * @author Karol Trzcinski
 * @date 2018-06-28
 */
#ifndef _BIN_PARSER_H
#define _BIN_PARSER_H

#include "../iassert.h"
#include "../mac/mac.h"
#include "../mac/mac_const.h"

/**
 * @brief connect BIN_ASSERT with IASSERT
 *
 */
#define BIN_ASSERT(expr) IASSERT(expr)

/**
 * @brief function code callback function template
 *
 * @param[in] data pointer to input data, first byte is function code (FC_),
 *   second byte is frame length
 * @param[in] info is pointer to extra frame information structure
 */
typedef void (*prot_parser_cb)(const void* data, const prot_packet_info_t* info);

/**
 * @brief struct of function code callbacks
 *
 */
typedef struct {
	FC_t FC;            ///< Function Code
	prot_parser_cb cb;  ///< associated callback function pointer
} prot_cb_t;

/// macro used to fill prot_cb_tab
/// then it will be easier to change text formating
#define ADD_FC(FC, CB) [FC] = { FC, CB }

/**
 * @brief parse single message
 *
 * @param[in] buf pointer to function code of message followed by frame len and
 * data
 * @param[in] info extra informations about frame
 * @return uint8_t frame len when it was processed, 0 otherwise
 */
uint8_t BIN_ParseSingle(const uint8_t* buf, const prot_packet_info_t* info);

/**
 * @brief parse each message in frame
 *
 * This function also keep buf->dPtr consistent so callback functions doesn't
 * have to manage buffer data pointer.
 *
 * @param[in] data buf pointer to buffer with message
 * @param[in] info extra informations about frame
 * @param size number of bytes to parse
 * @return uin8_t* pointer to data after processed messages
 */
const uint8_t* BIN_Parse(const uint8_t data[], const prot_packet_info_t* info, int size);

#endif  // _PROT_PARSER_H
