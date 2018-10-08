/**
 * @brief define numerical function codes for binary data
 *
 * @file bin_const.h
 * @author Karol Trzcinski
 * @date 2018-06-28
 */
#ifndef _BIN_CONST
#define _BIN_CONST

typedef enum {
	FC_BEACON = 0x00,   ///< beacon message is send when nobody sent message to
	                    ///< this device by a few seconds
	FC_TURN_ON = 0x01,  ///< when device turn on then it should send
	                    ///< turn on message
	FC_TURN_OFF = (FC_TURN_ON + 1),  ///< when device turn off then it
	                                 ///< should send turn off message
	FC_DEV_ACCEPTED = 0x03,          ///< device accepted in network by local sink
	FC_CARRY = 0x04,                 ///< carry message
	FC_FU = 0x05,                    ///< firmware upgrade message
	FC_SETTINGS_SAVE = 0x06,  ///< save current settings in non volatile memory
	FC_SETTINGS_SAVE_RESULT = 0x07,  ///< settings save result
	FC_RESET = 0x08,                 ///< reset device
	// empty
	FC_TOA_INIT = 0x10,               ///< to initialize toa routine
	FC_TOA_POLL = (FC_TOA_INIT + 1),  ///< first ranging message
	FC_TOA_RESP = (FC_TOA_INIT + 2),  ///< second ranging message
	FC_TOA_FIN = (FC_TOA_INIT + 3),   ///< third and last ranging message
	FC_TOA_RES = (FC_TOA_INIT + 4),   ///< ranging result message
	// empty
	// FC_SYNC_INIT = 0x16, ///<
	FC_SYNC_POLL = 0x17,  ///< first ranging message in sync procedure
	FC_SYNC_RESP = (FC_SYNC_POLL + 1),       ///< second ranging
	                                         ///< message in sync procedure
	FC_SYNC_FIN = (FC_SYNC_POLL + 2),        ///< third and last
	                                         ///< message in sync procedure
	FC_VERSION_ASK = 0x20,                   ///< device is asked about version
	FC_VERSION_RESP = (FC_VERSION_ASK + 1),  ///< device response version message
	// empty
	FC_STAT_ASK = 0x23,                ///< device is asked about status
	FC_STAT_RESP = (FC_STAT_ASK + 1),  ///< device is asked about status
	FC_STAT_SET = (FC_STAT_ASK + 2),   ///< device should change status to new one
	// empty
	FC_RFSET_ASK = 0x27,  ///< device is asked about transceiver settings
	FC_RFSET_RESP = (FC_RFSET_ASK + 1),  ///< device is asked
	                                     ///< about transceiver settings
	FC_RFSET_SET = (FC_RFSET_ASK + 2),   ///< device should change
	                                     ///< transceiver settings to new one
	// empty
	FC_BLE_ASK = 0x2B,
	FC_BLE_RESP = (FC_BLE_ASK + 1),
	FC_BLE_SET = (FC_BLE_ASK + 2),
	// empty
	// empty
	FC_IMU_ASK = 0x30,
	FC_IMU_RESP = (FC_IMU_ASK + 1),
	FC_IMU_SET = (FC_IMU_ASK + 2),
	//empty
	FC_BUZZ_ASK =  0x34,
} FC_t;

#endif
