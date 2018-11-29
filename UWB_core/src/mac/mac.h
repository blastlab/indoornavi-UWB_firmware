/**
 * @brief Medium Access Control layer 802.15.4
 *
 * @file mac.h
 * @author Karol Trzcinski
 * @date 2018-07-02
 *
 * This module is responsible for packets collision avoidance, retransmissions
 * and ack response sending.
 *
 * There should be timer enabled to tick every slot period to start transmission
 * routine in correct time (). For local time synchronization to global one is
 * responsible SYNC file.
 */
#ifndef _MAC_H
#define _MAC_H
#include "../parsers/bin_const.h"
#include "logs.h"
#include "transceiver.h"

#include "mac/mac_const.h"
#include "mac/mac_port.h"
#include "mac/mac_settings.h"
#include "mac/sync.h"

#define MAC_TRACE_ENABLED 0
#if MAC_TRACE_ENABLED
#define MAC_TRACE(...) LOG_DBG(__VA_ARGS__)
#else
#include "tools.h"
#define MAC_TRACE(...) ALL_UNUSED(__VA_ARGS__)
#endif

// macro to find unreleased buffer at compilation time
#define MAC_USAGE_BUF_START(name)  \
  mac_buf_t* #name = MAC_Buffer(); \
  if (#name != 0) {
#define MAC_USAGE_BUF_STOP(name) \
  MAC_Free(#name);               \
  }


#if DBG
#define TRACE_CNT 16
#endif

/**
 * @brief Trace function is usefull to track IRQ's behavior
 *
 * @param t
 */
void Trace(TRACE_t t);

/**
 * @brief mac protocol minimum head size
 *
 */
#define MAC_HEAD_LENGTH \
  (2 + 1 + sizeof(pan_dev_addr_t) + 2 * sizeof(dev_addr_t))

typedef struct {
	union {
		unsigned char buf[MAC_BUF_LEN];
		struct {
			unsigned char control[2];
			unsigned char seq_num;
			pan_dev_addr_t pan;
			dev_addr_t dst;
			dev_addr_t src;
			// unsigned int time;
			unsigned char data[64];
		}__packed frame;
	};
	unsigned char* dPtr;  ///< read/write pointer
	mac_buf_state state;  ///< current buf state
	int rx_len;           ///< number of received bytes includeing 80
	bool isRangingFrame;
	bool isServerFrame;         ///< send to server via USB or ETH
	short retransmit_fail_cnt;  ///< increased when after ack receive timeout
	unsigned int last_update_time;
} mac_buf_t;

typedef struct {
	int slot_number;
	mac_buf_t buf[MAC_BUF_CNT];
	uint8_t seq_num;
	short buf_get_ind;
	int64_t slot_time_offset;
	bool frame_under_tx_is_ranging;
	unsigned int last_rx_ts;
	unsigned int beacon_timer_timestamp;
} mac_instance_t;

/**
 * @brief mac data parser callback function typedef
 *
 */
typedef const uint8_t* (*MAC_DataParserCb_t)(const uint8_t data[], const prot_packet_info_t* info,
                                             int size);

/**
 * @brief mac send packet to sink function typedef
 *
 */
typedef void (*MAC_SendToSink_t)(const void* data, uint8_t size);

/**
 * @brief used by mac, externally implemented in platform folder
 *
 * @param[in] data full callback data
 */
void listener_isr(const dwt_cb_data_t* data);

/**
 * @brief initialize mac and transceiver
 *
 * @param[in] callback to binary data parser function
 * @param[in] callback sender function where target is sink
 */
void MAC_Init(MAC_DataParserCb_t callback, MAC_SendToSink_t sender);

/**
 * @brief call MAC_Init and do not change data parser callback
 *
 */
void MAC_Reinit();

/**
 * @brief configure receive to be in enable or disable state
 *
 * @param[in] receiver enable status to set
 */
void MAC_EnableReceiver(bool en);

/**
 * @brief return ms from last BeconTimerReset or received unicast message
 *
 * @return unsigned int ms from last BeconTimerReset or received unicast message
 */
unsigned int MAC_BeaconTimerGetMs();

/**
 * @brief reset beacon timer after sending a beacon message
 *
 */
void MAC_BeaconTimerReset();

/**
 * @brief send beacon message
 */
void MAC_BeaconSend();

/**
 * @brief time converter
 *
 * @param glob_time in Dwt time units
 * @return int slot time in us
 */
int MAC_ToSlotsTime(int64_t glob_time);

/**
 * @brief should be called at the beginning of your slot time
 *
 */
void MAC_YourSlotIsr();

/**
 * @brief release buffer waiting for this ack
 *
 * @param seq_num frame sequence number
 */
void MAC_AckFrameIsr(uint8_t seq_num);

/**
 * @brief reserve buffer
 *
 * @return mac_buf_t* result buffer pointer or zero
 */
mac_buf_t* MAC_Buffer();

/**
 * @brief return length of data already written in frame
 *
 * @param[in] buf buffer to analyse
 * @return int return length of data already written in frame
 */
int MAC_BufLen(const mac_buf_t* buf);

/**
 * @brief low level function, used only by carry module
 *
 * fill 802.15.4 fields
 *
 * @param[in,out] buf
 * @param[in] target device address
 */
void MAC_FillFrameTo(mac_buf_t* buf, dev_addr_t target);

/**
 * @brief set frame type in 802.15.4 protocol
 *
 * @param[in,out] buf target buffer
 * @param[in] FR_CR_type FC_CR_xx macro
 */
void MAC_SetFrameType(mac_buf_t* buf, uint8_t FR_CR_type);

/**
 * @brief reserve buffer and fill mac protocol fields
 *
 * @param[in] target address to target device in range of radio (without hops)
 * @param[in] can_append true if can appended to other packet to this target
 * @return mac_buf_t* result buffer with filled fields or zero
 */
mac_buf_t* MAC_BufferPrepare(dev_addr_t target, bool can_append);

/**
 * @brief release buffer
 *
 * @param[in] buf buffer to release
 */
void MAC_Free(mac_buf_t* buf);

/**
 * @brief add frame to the transmit queue
 *
 * It is default way to send data packets.
 * Buf will be released after transmission
 *
 * @param[in] buf with frame
 * @param[in] ack_require
 */
void MAC_Send(mac_buf_t* buf, bool ack_require);

/**
 * @brief send packet as a ranging frame
 *
 * @param buf to transmit
 * @param transceiver_flags:
 *   - DWT_START_TX_IMMEDIATE
 *   - DWT_START_TX_DELAYED
 *   - DWT_RESPONSE_EXPECTED
 * @return int 0 if succeed or error code
 */
int MAC_SendRanging(mac_buf_t* buf, uint8_t transceiver_flags);

/**
 * @brief return time in ms from last received packed
 *
 * @return unsigned int time in ms from last received packed
 */
unsigned int MAC_UsFromRx();

/**
 * @brief read one byte from frame and move rw pointer
 *
 *
 * @param[in] frame to read
 * @return unsigned char
 */
unsigned char MAC_Read8(mac_buf_t* frame);

/**
 * @brief write one byte from frame and move rw pointer
 *
 *
 * @param[in,out] frame to write
 * @param value
 */
void MAC_Write8(mac_buf_t* frame, unsigned char value);

/**
 * @brief read data chunk from frame and move rw pointer
 *
 * @param[in] frame to read
 * @param[out] destination address
 * @param[in] len number of bytes to write
 */
void MAC_Read(mac_buf_t* frame, void* destination, unsigned int len);

/**
 * @brief write chunk of bytes from frame and move rw pointer
 *
 * @param[in,out] frame to write
 * @param[in] src address of data to write
 * @param[in] len number of bytes to write
 */
void MAC_Write(mac_buf_t* frame, const void* src, unsigned int len);

int MAC_TryTransmitFrame();

#endif  // _MAC_H
