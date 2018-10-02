/**
 * @file logs_err.h
 * @author Karol Trzcinski
 * @brief error logs definition
 * @date 2018-10-02
 * 
 * @copyright Copyright (c) 2018
 * 
 */
// network
ADD_ITEM_M(1101, ERR_MAC_NO_MORE_BUFFERS, "No more buffers")
ADD_ITEM_M(1102, ERR_MAC_BAD_OPCODE_LEN, "%s bad len %d!=%d")
ADD_ITEM_M(1103, ERR_BAD_OPCODE_LEN, "%s bad len %d!=%d")
ADD_ITEM_M(1104, ERR_BAD_OPCODE, "unknown opcode %Xh")
ADD_ITEM_M(1105, ERR_PARENT_FOR_SINK, "parent can't be set for sink")
ADD_ITEM_M(1106, ERR_PARENT_NEED_ANCHOR, "parent must be an anchor (%X)")

// radio 1200
ADD_ITEM_M(1201, ERR_RF_BAD_CHANNEL, "rfset ch 1..7 (without 6)")
ADD_ITEM_M(1202, ERR_RF_BAD_BAUDRATE, "rfset br 110/850/6800")
ADD_ITEM_M(1203, ERR_RF_BAD_PREAMBLE_LEN, "rfset plen 64/128/256/512/1024/1536/2048/4096")
ADD_ITEM_M(1204, ERR_RF_BAD_PRF, "rfset prf 16/64")
ADD_ITEM_M(1205, ERR_RF_BAD_PAC, "rfset pac 8/16/32/64")
ADD_ITEM_M(1206, ERR_RF_BAD_CODE, "rfset code 1..24")
ADD_ITEM_M(1207, ERR_RF_BAD_NSSFD, "rfset nssfd 0/1")
ADD_ITEM_M(1208, ERR_BLE_INACTIVE, "BLE is disabled")

// ranging 1300
ADD_ITEM_M(1301, ERR_MEASURE_FAILED_DID, "measure failed after %X")
ADD_ITEM_M(1302, ERR_MEASURE_FAILED_ANC_CNT, "measure failed ancCnt:%d")
ADD_ITEM_M(1303, ERR_SETANCHORS_FAILED, "setanchors failed (%X)")
ADD_ITEM_M(1304, ERR_SETTAGS_NEED_SETANCHORS, "settags need setanchors")
ADD_ITEM_M(1305, ERR_SETTAGS_FAILED, "settags failed after %X")

// settings 1400
ADD_ITEM_M(1401, ERR_FLASH_ERASING, "flash erasing error did:%X")
ADD_ITEM_M(1402, ERR_FLASH_WRITING, "flash writing error did:%X")
ADD_ITEM_M(1403, ERR_FLASH_OTHER, "SETTINGS_Save bad implementation did:%X")

// other 1500
ADD_ITEM_M(1501, ERR_BAD_COMMAND, "Bad command")
ADD_ITEM_M(1502, ERR_BASE64_TOO_LONG_INPUT, "TXT_Bin too long base64 message")
ADD_ITEM_M(1503, ERR_BASE64_TOO_LONG_OUTPUT, "LOG_Bin too long base64 message, FC:%xh")