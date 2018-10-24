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
ARG("%s", "function code name")
ARG("%d", "received length")
ARG("%d", "expected length")

ADD_ITEM_M(1103, ERR_BAD_OPCODE_LEN, "%s bad len %d!=%d")
ARG("%s", "function code name")
ARG("%d", "received length")
ARG("%d", "expected length")

ADD_ITEM_M(1104, ERR_BAD_OPCODE, "unknown opcode %Xh")
ADD_ITEM_M(1105, ERR_PARENT_FOR_SINK, "parent can't be set for sink")

ADD_ITEM_M(1106, ERR_PARENT_NEED_ANCHOR, "parent must be an anchor (%X)")
ARG("%X", "address of incorrect device")

// radio 1200
ADD_ITEM_M(1201, ERR_RF_BAD_CHANNEL, "rfset ch 1..7 (without 6)")
ADD_ITEM_M(1202, ERR_RF_BAD_BAUDRATE, "rfset br 110/850/6800")
ADD_ITEM_M(1203, ERR_RF_BAD_PREAMBLE_LEN, "rfset plen 64/128/256/512/1024/1536/2048/4096")
ADD_ITEM_M(1204, ERR_RF_BAD_PRF, "rfset prf 16/64")
ADD_ITEM_M(1205, ERR_RF_BAD_PAC, "rfset pac 8/16/32/64")
ADD_ITEM_M(1206, ERR_RF_BAD_CODE, "rfset code 1..24")
ADD_ITEM_M(1207, ERR_RF_BAD_NSSFD, "rfset nssfd 0/1")

ADD_ITEM_M(1208, ERR_RF_TX_NEED_COARSE_AND_FINE_P, "txset need P%dc and P%df at the same time")
ARG("%d", "number of P argument")
ARG("%d", "number of P argument")

ADD_ITEM_M(1209, ERR_RF_TX_BAD_COARSE_P, "txset P%dc must be divisible by 3 and <=18")
ARG("%d", "number of P argument")

ADD_ITEM_M(1210, ERR_RF_TX_BAD_FINE_P, "txset P%df must be <=31")
ARG("%d", "number of P argument")

ADD_ITEM_M(1211, ERR_BLE_INACTIVE, "BLE is disabled")
COMMENT("BLE module is not included into this version of firmware")

ADD_ITEM_M(1212, ERR_BLE_BAD_TXPOWER, "Wrong ble txpower value")
COMMENT("BLE module is not included into this version of firmware")

// ranging 1300
ADD_ITEM_M(1301, ERR_MEASURE_ADD_ANCHOR_FAILED_DID, "measure add anchor failed with %X")
ARG("hex", "incorrect anchor address")

ADD_ITEM_M(1302, ERR_MEASURE_TARGET_WITH_ANC_FAILED, "measure target failed ancCnt:%d")
ARG("ancCnt", "number of anchors to connect with target")


ADD_ITEM_M(1303, ERR_SETANCHORS_FAILED, "setanchors failed (%X)")
ARG("hex", "address of device which cause error")

ADD_ITEM_M(1304, ERR_SETTAGS_NEED_SETANCHORS, "settags need setanchors")

ADD_ITEM_M(1305, ERR_SETTAGS_FAILED, "settags failed after %X")
ARG("hex", "address of device which cause error")

ADD_ITEM_M(1306, ERR_MAC_RAAD_BAD_VALUE, "mac raad value must be 0 or 1 (enable)")
ADD_ITEM_M(1307, ERR_MAC_ADDR_BAD_VALUE, "mac addr bad value")
ADD_ITEM_M(1308, ERR_MAC_BEACON_TIMER_PERIOD_TOO_SHORT, "mac beacon period must be greater than %d")
ARG("%d", "minumum beacon period value")

// settings 1400
ADD_ITEM_M(1401, ERR_FLASH_ERASING, "flash erasing error did:%X")
ADD_ITEM_M(1402, ERR_FLASH_WRITING, "flash writing error did:%X")
ADD_ITEM_M(1403, ERR_FLASH_OTHER, "SETTINGS_Save bad implementation did:%X")

// other 1500
ADD_ITEM_M(1501, ERR_BAD_COMMAND, "Bad command")
ADD_ITEM_M(1502, ERR_BASE64_TOO_LONG_INPUT, "TXT_Bin too long base64 message")

ADD_ITEM_M(1503, ERR_BASE64_TOO_LONG_OUTPUT, "LOG_Bin too long base64 message, FC:%xh")
ARG("FC", "hexadecimal function code which cause error")
