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
  uint8_t FC, len;
  unsigned char flags;  ///< extra flags field, CARRY_FLAG_xx
  unsigned char verHopsNum;  ///< version (upper nibble) hops number (lower nibble)
  dev_addr_t src_addr;  ///< source address
  dev_addr_t hops[0];  ///< destination .. next hop
} FC_CARRY_s;

/**
 * @brief carry head minimal length
 * 
 */
#define CARRY_HEAD_MIN_LEN (1 + 1 + sizeof(dev_addr_t) + 1)

/**
 * @brief used in CARRY_PrepareBufTo
 * 
 */
#define CARRY_ADDR_SINK 0

/**
 * @brief used in CARRY_PrepareBufTo
 * 
 */
#define CARRY_ADDR_SERVER ADDR_BROADCAST


/**
 * @brief global singleton
 * 
 */
typedef struct {
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
 * @brief return address of parent anchor
 * 
 * Each message in sink or server direction will be
 * send via this device.
 * 
 * @return dev_addr_t address of parent anchor
 */
dev_addr_t CARRY_ParentAddres();

/**
 * @brief set new device parent
 * 
 * @param target device address
 * @param parent new target address
 */
bool CARRY_ParentSet(dev_addr_t target, dev_addr_t parent);

/**
 * @brief get current device parent
 * 
 * @param target device address 
 * @return dev_addr_t parent of this device
 */
dev_addr_t CARRY_ParentGet(dev_addr_t target);

/**
 * @brief delete each saved parent
 *
 */
void CARRY_ParentDeleteAll();

/**
 * @brief write trace to target, including target address
 * 
 * target address is the last one
 * 
 * @param[in] buf address to write trace
 * @param[in] target 
 * @return int hops counter 0..#CARRY_MAX_HOPS
 */
int CARRY_WriteTrace(uint8_t *buf, dev_addr_t target, dev_addr_t* nextDid);


/**
 * @brief reserve buffer, write headers and set buffer fields default values.
 * 
 * @param[in] target device address
 * @param[out] resulting buffer carry pointer
 * @return mac_buf_t* result buffer or null
 */
mac_buf_t *CARRY_PrepareBufTo(dev_addr_t target, FC_CARRY_s** out_pcarry);



/**
 * @brief send buffer via UWB or USB to correct device
 * 
 * @param buf filled buffer data to send
 */
void CARRY_Send(mac_buf_t* buf, bool ack_req);


/**
 * @brief function called from MAC module after receiving CARRY frame
 * 
 * @param[in] data pointer to data
 * @param[in] info about frame
 */
void CARRY_ParseMessage(const void *data, const prot_packet_info_t *info);


/**
 * @brief read one byte from frame and move rw pointer
 * 
 * 
 * @param[in] frame to read
 * @return unsigned char 
 */
unsigned char CARRY_Read8(mac_buf_t *frame);


/**
 * @brief write one byte from frame and move rw pointer
 * 
 * 
 * @param[in,out] pointer to buffer carry structure
 * @param[in,out] frame to write
 * @param value 
 */
void CARRY_Write8(FC_CARRY_s* carry, mac_buf_t *frame, unsigned char value);


/**
 * @brief read data chunk from frame and move rw pointer
 * 
 * @param[in] frame to read
 * @param[out] destination address
 * @param[in] len number of bytes to write
 */
void CARRY_Read(mac_buf_t *frame, void *destination, unsigned int len);


/**
 * @brief write chunk of bytes from frame and move rw pointer
 * 
 * @param[in,out] pointer to buffer carry structure
 * @param[in,out] frame to write
 * @param[in] src address of data to write
 * @param[in] len number of bytes to write
 */
void CARRY_Write(FC_CARRY_s* carry, mac_buf_t *frame, const void *src, unsigned int len);

#endif // _CARRY_H
