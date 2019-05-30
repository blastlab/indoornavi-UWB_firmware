/**
 * @brief carry is a transport/routing layer
 *
 * module responsible for writing trace to the target device
 *
 * @file carry.h
 * @author Karol Trzcinski
 * @date 2018-07-02
 * 
 * 
 * # CARRY protocol
 * 
 * ## Goal
 * 
 * This layer is responsible for smoothly sending messages between
 * anchors, tags, sink and server (via USB/UART/ETH). Messages between
 * device diffrent from sink to another device is forbidden, it must be 
 * passed through sink or even server. Also it is responsible
 * for routing messages between devices. So it is a network layer in ISO/OSI.
 * 
 * 
 * ## Details
 * 
 * 
 * ### Overview
 * 
 * There are two meta addresses CARRY_ADDR_SINK and CARRY_ADDR_SERVER.
 * Server is defined as device connected to sink gateway.
 * 
 * 
 * ### Routing organisation 
 * 
 * Devices are organized in a little modified star topology. Sink is in center,
 * each device can send message to sink and sink can send message to any device.
 * Also through hops in both directions.
 * 
 * Each device remember his parent. Parent is change after receiving frame with 
 * flag CARRY_FLAG_REFRESH_PARENT. Such a messages are sent as usual with 
 * Firmware Upgrade, device accept frames and frames sent from server.
 * 
 * In opposite direction there is need to remember whole route in sink memory.
 * Carry settings has list of anchors to remember and them parent. Modification
 * of this list can be made from text command or when in carry settings autoRoute
 * is set to true then parent is set after receiving BEACON message from target
 * device. Before new parent is set, number of hops between old and new parent
 * are check and time parent inactive time. 
 * 
 * /todo: update carry target lastUpdateTime
 * 
 * For tag's it is similar than for anchors, there are no any condition during
 * assignment to anchor. Tracking is involved during parsing measurements
 * in program main loop and during parsing beacon message.
 * 
 * 
 * ### Lower layers
 * 
 * CARRY use MAC module as an UWB tranport layer and LOG_Bin to send message to
 * gateway.
 * 
 * 
 * ### Messages 
 * 
 * 
 * #### CARRY -- CARRY protocol -- Sink/Server->Device Device->Sink/Server
 * 
 * [FC:1][frameLen:1][flags:1][verHopsNum:4][src_addr:2][hops:n]
 *
 * - FC						- FC_CARRY constant
 * - frameLen 		- length a whole carry message, including content
 * - flags				- flags with CARRY_FLAG_ prefix
 * - verHopsNum		- hi nibble is a protocol version and lower nibble is a hops number
 * 									(number of addresses in hops array)
 * - src_addr			- address of source device
 * - hops					- array of device addresses for hops
 * 
 */

#ifndef _CARRY_H
#define _CARRY_H

#include <string.h>  // memcpy

#include "../mac/mac.h"
#include "../settings.h"

#include "carry_const.h"
#include "carry_settings.h"

#include "../parsers/bin_parser.h"

/**
 * @brief protocol struct
 *
 */
typedef struct {
	uint8_t FC, len;
	unsigned char flags;  ///< extra flags field, CARRY_FLAG_xx
	unsigned char verHopsNum;       ///< version (upper nibble) hops number (lower nibble)
	dev_addr_t src_addr;  ///< source address
	dev_addr_t hops[0];   ///< destination .. next hop
}__packed FC_CARRY_s;

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

typedef struct
{
	time_ms_t updateTime;
	dev_addr_t did;
	dev_addr_t anchor;
} carry_tag_t;

/**
 * @brief global singleton
 *
 */
typedef struct {
	bool isConnectedToServer;
	dev_addr_t toSinkId;
	carry_tag_t tags[CARRY_MAX_TAGS];
} carry_instance_t;

/**
 * @brief initialize module data
 *
 * @param[in] isConnectedToServer bool value indicating device connection with
 *   remote server
 */
void CARRY_Init(bool isConnectedToServer);

/**
 * @brief track tag positions - connect tag address with anchor did
 *
 * @param[in] tag_did tag address
 * @param[in] parent currently connected anchor address
 * @return 0 when it is new tag and there is no place for him
 * @return 1 when new connection has been established
 * @return 2 when new connection is same as an old one
 */
int CARRY_TrackTag(dev_addr_t tag_did, dev_addr_t parent);

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
 * @brief set address in sink direction
 *
 */
void CARRY_SetYourParent(dev_addr_t did);

/**
 * @brief set new device parent
 *
 * @param target device address
 * @param parent new target address
 * @return 0 when fail
 * @return 1 when current parent is identical
 * @return 2 when new parent has been rejected because of tree level
 * @return 3 when parent changed
 * @return 4 when target created with a given parent
 */
int CARRY_ParentSet(dev_addr_t target, dev_addr_t parent);

/**
 * @brief get current device parent
 *
 * @param target device address
 * @return dev_addr_t parent of this device
 */
dev_addr_t CARRY_ParentGet(dev_addr_t target);

/**
 * @brief get number of hops to target (from sink)
 *
 * @param target device address
 * @return int number o hops from sink
 */
int CARRY_GetTargetLevel(dev_addr_t target);

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
 * @param[out] nextDid address of next device, where packet should be sent
 * @return int hops counter 0..#CARRY_MAX_HOPS
 */
int CARRY_WriteTrace(uint8_t* buf, dev_addr_t target, dev_addr_t* nextDid);

/**
 * @brief reserve buffer, write headers and set buffer fields default values.
 *
 * @param[in] target device address
 * @param[out] out_pcarry resulting buffer carry pointer
 * @return mac_buf_t* result buffer or null
 */
mac_buf_t* CARRY_PrepareBufTo(dev_addr_t target, FC_CARRY_s** out_pcarry);

/**
 * @brief send buffer via UWB or USB to correct device
 *
 * @param[in,out] buf filled buffer data to send
 * @param[in] ack_req say that frame need ack packet after reception or not
 */
void CARRY_Send(mac_buf_t* buf, bool ack_req);

/**
 * @brief function called from MAC module after receiving CARRY frame
 *
 * @param[in] data pointer to data
 * @param[in] info about frame
 */
void CARRY_ParseMessage(const void* data, const prot_packet_info_t* info);

/**
 * @brief read one byte from frame and move rw pointer
 *
 * @param[in] frame to read
 * @return unsigned char
 */
unsigned char CARRY_Read8(mac_buf_t* frame);

/**
 * @brief write one byte from frame and move rw pointer
 *
 * @param[in,out] carry pointer to buffer carry structure
 * @param[in,out] frame to write
 * @param[in] value to write
 */
void CARRY_Write8(FC_CARRY_s* carry, mac_buf_t* frame, unsigned char value);

/**
 * @brief read data chunk from frame and move rw pointer
 *
 * @param[in] frame to read
 * @param[out] destination address
 * @param[in] len number of bytes to write
 */
void CARRY_Read(mac_buf_t* frame, void* destination, unsigned int len);

/**
 * @brief write chunk of bytes from frame and move rw pointer
 *
 * @param[in,out] carry pointer to buffer carry structure
 * @param[in,out] frame to write
 * @param[in] src address of data to write
 * @param[in] len number of bytes to write
 */
void CARRY_Write(FC_CARRY_s* carry, mac_buf_t* frame, const void* src, unsigned int len);

#endif  // _CARRY_H
