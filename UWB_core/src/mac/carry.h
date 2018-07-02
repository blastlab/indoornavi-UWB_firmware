/**
 * @brief carry is a transport/routing layer
 * 
 * module responsible for writing trace to the target device
 * 
 * @file carry.h
 * @author Karol Trzcinski
 * @date 2018-07-02
 */
#ifndef _CARRY_H
#define _CARRY_H

#include <string.h> // memcpy

#include "../mac/mac.h"
#include "../settings.h"

#include "carry_const.h"
#include "carry_settings.h"

#include "../parsers/bin_parser.h"


/**
 * @brief protocol struct
 * 
 */
typedef struct __packed {
  unsigned char flag_hops;  ///< extra flags field, CARRY_FLAG_xx
  dev_addr_t src_addr;  ///< source address
  dev_addr_t hops[0];  ///< destination .. next hop
} FC_CARRY_s;

/**
 * @brief carry head minimal length
 * 
 */
#define CARRY_HEAD_MIN_LEN (1 + 1 + sizeof(dev_addr_t) + 1)


/**
 * @brief data about some trace to target
 * @deprecated use anchor->parent model
 * 
 */
typedef struct {
  dev_addr_t path[CARRY_MAX_HOPS];  ///< trace path
  int path_len;  ///< number of hops in path
  int pass_cnt;  ///< number of successful packet transmission via this path
  int fail_cnt;  ///< number of failed packet transmission via this path
  mac_buff_time_t last_update_time;  ///< trace last active time
} carry_trace_t;


/**
 * @brief target info struct
 * 
 */
typedef struct {
  dev_addr_t addr;
  carry_trace_t trace[CARRY_MAX_TRACE];
  mac_buff_time_t last_update_time;
} carry_target_t;


/**
 * @brief global singleton
 * 
 */
typedef struct {
  carry_target_t target[CARRY_MAX_TARGETS];
  bool isConnectedToServer;
  dev_addr_t toSinkId;
} carry_instance_t;


/**
 * @brief initialize module data
 * 
 * @param[in] isConnectedToServer bool value indicating device connection with
 *   remote server
 */
void CARRY_Init(bool isConnectedToServer);


/**
 * @brief write trace to target, including target address
 * 
 * target address is the last one
 * 
 * @param[in] buf 
 * @param[in] target 
 * @return int number of written addresses or 0 when target is unknown
 */
int CARRY_WriteTrace(dev_addr_t *buf, dev_addr_t target);


/**
 * @brief reserve buffer, write headers and set buffer fields default values.
 * 
 * @param[in] target device address
 * @return mac_buf_t* result buffer or null
 */
mac_buf_t *CARRY_PrepareBufTo(dev_addr_t target);


/**
 * @brief find or create buffer to the target device
 * 
 * returned buffer can be partially filled
 * 
 * @param[in] target addres
 * @return mac_buf_t* result buffer or null
 */
mac_buf_t *CARRY_GetBufTo(dev_addr_t target);


/**
 * @brief function called from MAC module after receiving CARRY frame
 * 
 * @param[in] buf 
 */
void CARRY_ParseMessage(mac_buf_t *buf);

#endif // _CARRY_H
