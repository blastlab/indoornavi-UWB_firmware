/**
 * @file logs_wrn.h
 * @author Karol Trzcinski
 * @brief warning logs definition
 * @date 2018-10-02
 * 
 * @copyright Copyright (c) 2018
 * 
 */
// network work time events
ADD_ITEM_M(1101, WRN_CARRY_INCOMPATIBLE_VERSION, "CARRY incompatible version %d (%d)")
ADD_ITEM_M(1102, WRN_CARRY_TARGET_NOBODY, "CARRY target nobody")
ADD_ITEM_M(1103, WRN_MAC_FRAME_BAD_OPCODE, "MAC frame with bad opcode %X")
ADD_ITEM_M(1104, WRN_MAC_UNSUPPORTED_MAC_FRAME, "MAC unsupported frame type %X")
ADD_ITEM_M(1105, WRN_MAC_UNSUPPORTED_ACK_FRAME, "MAC ack frame is not supported yet")
ADD_ITEM_M(1108, WRN_FIRWARE_NOT_ACCEPTED_YET, "new firmware not accepted yet! did:%X")
ADD_ITEM_M(1109, WRN_SINK_ACCEPT_SINK, "sink can't have any parent")

// radio 1200
ADD_ITEM_M(1201, WRN_MAC_TX_ERROR, "Tx err")
ADD_ITEM_M(1202, WRN_MAC_TOO_BIG_FRAME, "Frame with size %d can't be send within %dus slot")

// ranging 1300
ADD_ITEM_M(1301, WRN_RANGING_TOO_SMALL_PERIOD, "Too small period! Now N:%d T:%d")

// settings 1400

// others 1500
