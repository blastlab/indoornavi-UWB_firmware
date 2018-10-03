/**
 * @file logs_inf.h
 * @author Karol Trzcinski
 * @brief informational logs definition
 * @date 2018-10-02
 *
 * @copyright Copyright (c) 2018
 *
 */

// network
ADD_ITEM_M(1101, INF_DEVICE_TURN_ON, "Device turn on did:%X fV:%d")
ADD_ITEM_M(1102, INF_DEVICE_TURN_OFF, "Device turn off did:%X")
ADD_ITEM_M(1103, INF_BEACON, "Beacon from did:%X")
ADD_ITEM_M(1104, INF_DEV_ACCEPTED, "Device accepted, sink:%X parent:%X")
ADD_ITEM_M(1106, INF_PARENT_DESCRIPTION, "parent of %X is %X (%d)")
ADD_ITEM_M(1107, INF_PARENT_SET, "parent %X for %d devices, failed for %d")
ADD_ITEM_M(1108, INF_PARENT_CNT, "parent cnt:%d")
ADD_ITEM_M(1111, INF_STATUS, "stat did:%x mV:%d Rx:%d Tx:%d Er:%d To:%d Uptime:%dd.%dh.%dm.%ds")
ADD_ITEM_M(1112, INF_VERSION, "version %X %s %d.%d %d.%d.%X")
ADD_ITEM_M(1113, INF_ROUTE, "route auto:%d")

// radio
ADD_ITEM_M(1201, INF_RF_SETTINGS, "rfset ch:%d-%d/%d br:%d plen:%d prf:%d pac:%d code:%d nsSfd:%d sfdTo:%d")
ADD_ITEM_M(1202, INF_BLE_SETTINGS,  "ble txpower:%d (-40/-20/-16/-12/-8/-4/0/3/4) enable:%d (0/1) did:%X")

// ranging
ADD_ITEM_M(1301, INF_MEASURE_DATA, "a %X>%X %d %d %d %d")
ADD_ITEM_M(1302, INF_MEASURE_INFO, "measure %X with [%s]")
ADD_ITEM_M(1303, INF_MEASURE_CMD_CNT, "measure cnt:%d")
ADD_ITEM_M(1304, INF_MEASURE_CMD_SET, "measure set %X with %d anchors")
ADD_ITEM_M(1305, INF_RANGING_TIME, "rangingtime T:%d t:%d (N:%d)")
ADD_ITEM_M(1306, INF_TOA_SETTINGS, "%s gt:%d fin:%d resp1:%d resp2:%d")
COMMENT("prefix is dependent of usage")
ADD_ITEM_M(1307, INF_CLEARED_PARENTS, "cleared parents")
ADD_ITEM_M(1308, INF_CLEARED_MEASURES, "cleared measures")
ADD_ITEM_M(1309, INF_CLEARED_PARENTS_AND_MEASURES, "cleared parents and measures")
ADD_ITEM_M(1310, INF_CLEAR_HELP, "clear [-m,-p,-mp]")
ADD_ITEM_M(1311, INF_SETANCHORS_SET, "setanchors set %d anchors")
ADD_ITEM_M(1312, INF_SETTAGS_SET, "settags set %d tags with %d anchors")
ADD_ITEM_M(1313, INF_DELETETAGS, "deletetags deleted %d tags")

// settings
ADD_ITEM_M(1401, INF_SETTINGS_SAVED, "settings saved did:%X")
ADD_ITEM_M(1402, INF_SETTINGS_NO_CHANGES, "no changes to be saved did:%X")

// others
ADD_ITEM_M(1501, INF_IMU_SETTINGS, "imu delay:%d enable:%d (0/1) did:%X")
ADD_ITEM_M(1502, INF_FU_SUCCESS, "Firmware upgrade success")
