/**
 * @file logs_inf.h
 * @author Karol Trzcinski
 * @brief informational logs definition
 * @date 2018-10-02
 *
 * @copyright Copyright (c) 2018
 *
 */

//
// network
//
ADD_ITEM_M(1101, INF_DEVICE_TURN_ON, "Device turn on did:%X fV:%d")
ARG("fV", "firmware major version")

ADD_ITEM_M(1102, INF_DEVICE_TURN_OFF, "Device turn off did:%X")

ADD_ITEM_M(1103, INF_BEACON, "Beacon from did:%X mV:%d route:[%s] serial:%X%08X ")
ARG("mV", "device voltage on accumulator connectors")
ARG("route", "message route in format '[%X>%X>%X...] where last position is target address")
ARG("serial", "is device 64B or 48B uniqual number, may be build based on device MAC address, especially if device has  BlueTooth functionality")

ADD_ITEM_M(1104, INF_DEV_ACCEPTED, "Device accepted, sink:%X parent:%X")
ARG("sink", "hex address of parent sink")
ARG("parent", "hex address of parent device (in sink direction)")

ADD_ITEM_M(1106, INF_PARENT_DESCRIPTION, "parent of c:%X is p:%X # n:%d")
ARG("c", "hex child did")
ARG("p", "hex parent did, parent for sink is always 0")
ARG("n", "number of child hops to sink")
COMMENT("see :ref:`parent`")

ADD_ITEM_M(1107, INF_PARENT_SET, "parent p:%X for s:%d devices, failed for f:%d")
ARG("p", "hex parent did")
ARG("s", "number of devices, where parent has been set to a given one")
ARG("f", "number of devices, where parent has not been set to a given one")
COMMENT("see :ref:`parent`")

ADD_ITEM_M(1108, INF_PARENT_CNT, "parent cnt:%d")
ARG("cnt", "number of saved parent in volatile memory")
COMMENT("see :ref:`parent`")

ADD_ITEM_M(1111, INF_STATUS, "stat did:%x mV:%d Rx:%d Tx:%d Er:%d To:%d Uptime:%dd.%dh.%dm.%ds")
ARG("mV", "battery voltage in millivolts, correct values for devices with battery is 3100..4300, for devices without battery 4500..5200")
ARG("Rx", "12-bits counter of correctly received packets, values in range 0..4095. After overflow it will count from 0.")
ARG("Tx", "12-bits counter of correctly transmitted packets, values in range 0..4095. After overflow it will count from 0.")
ARG("Er", "12-bits counter of receiving packets error, values in range 0..4095. After overflow it will count from 0.")
ARG("To", "12-bits counter of timeout during transmitting or receiving frames, values in range 0..4095. After overflow it will count from 0.")
ARG("Uptime", "device work time in format days.hours.minuts.seconds. It overflow ofter 49.7 days.")
COMMENT("see :ref:`status`")

ADD_ITEM_M(1112, INF_VERSION, "version did:%X serial:%X%08X r:%s hV:%d.%d.%d fV:%d.%d.%X%X")
ARG("serial", "64-bit device unique identificator number")
ARG("r", "device role, possible values {SINK, ANCHOR, TAG, LISTENER, DEFAULT, OTHER}")
ARG("hV", "hardware version, major.minor.type")
ARG("fV", "formware version major.minor.hash where source repository commit hash is in hexadecimal and is 32-bit value.")
COMMENT("see :ref:`version`")

ADD_ITEM_M(1113, INF_ROUTE, "route auto:%d")
ARG("auto", "automaticaly route module status {0-off, 1-on}")
COMMENT("see :ref:`route`")

ADD_ITEM_M(1114, INF_MAC, "mac did:%X pan:%X beacon:%d sp:%d st:%d gt:%d raad:%d role:%s")
ARG("pan", "personal area network identifier")
ARG("beacon", "interval in :math:`ms`")
ARG("sp", "slot period in :math:`\mu s`")
ARG("st", "one slot time in :math:`\mu s`")
ARG("gt", "slot guard time in :math:`\mu s`")
ARG("raad", "raport anchor to anchor distances boolean")
ARG("role", "device role, possible values {SINK, ANCHOR, TAG, LISTENER, DEFAULT, OTHER}")

//
// radio
//
ADD_ITEM_M(1201, INF_RF_SETTINGS, "rfset ch:%d-%d/%d br:%d plen:%d prf:%d pac:%d code:%d nsSfd:%d sfdTo:%d smartTx:%d")
ARG("ch", "channel number - (frequency/bandwidth")
ARG("br", "baudrate in kbps")
ARG("plen", "preamble length")
ARG("prf", "pulse repetition frequency in MHz")
ARG("pac", "preamble acquisition chunk size")
ARG("code", "communication code")
ARG("nsSfd", "non standard frame delimiter {0-off, 1-on}")
ARG("sfdTo", "SFD detection timeout count")
ARG("smartTx", "smart tx booster for short messages {0-off, 1-on}")
COMMENT("see :ref:`rfset`")

ADD_ITEM_M(1202, INF_RF_TX_SETTINGS, "txset did:%X pgdly:%d P1:%d+%d.%d P2:%d+%d.%d P3:%d+%d.%d P4:%d+%d.%d")
ARG("pgdly", "power generator delay")
ARG("P1", "power gain in db for shoertest messages (<0.125ms)")
ARG("P2", "power gain in db for short messages (<0.25ms)")
ARG("P3", "power gain in db for long messages (<0.5ms")
ARG("P4", "power gain in db for longest mesages (>=0.5ms)")
COMMENT("In smart tx power is disabled, then only P4 is used")

ADD_ITEM_M(1203, INF_BLE_SETTINGS, "ble txpower:%d (-40/-20/-16/-12/-8/-4/0/3/4) enable:%d (0/1) did:%X")
ARG("txpower", "ble transmitter power settings")
ARG("enable", "bluetooth module status")
COMMENT("see :ref:`ble`")

//
// ranging
//
ADD_ITEM_M(1301, INF_MEASURE_DATA, "a %X>%X %d %d %d %d")
ARG("first", "hex did of first device")
ARG("second", "hex did of second device")
ARG("third", "distance in cm")
ARG("fourth", "RSSI in dBm*100")
ARG("fifth", "FPP in dBm*100")
COMMENT("see :ref:`measure`")

ADD_ITEM_M(1302, INF_MEASURE_INFO, "measure t:%X with a:[%s]")
ARG("t", "hex target device address")
ARG("a", "list of hex anchors addresses in one measure")
COMMENT("see :ref:`measure`")

ADD_ITEM_M(1303, INF_MEASURE_CMD_CNT, "measure cnt:%d")
ARG("cnt", "measure counter in volatile memory")
COMMENT("see :ref:`measure`")

ADD_ITEM_M(1304, INF_MEASURE_CMD_SET, "measure set t:%X with cnt:%d anchors")
ARG("t", "hex target device address")
ARG("cnt", "number of new measures")
COMMENT("see :ref:`measure`")

ADD_ITEM_M(1305, INF_RANGING_TIME, "rangingtime T:%d t:%d (N:%d)")
ARG("T", "ranging period")
ARG("t", "delay between ranging")
ARG("N", "number of ranging slot in a given period")
COMMENT("see :ref:`rangingtime`")

ADD_ITEM_M(1306, INF_TOA_SETTINGS, "%s gt:%d fin:%d resp1:%d resp2:%d")
ARG("first", "usage dependant prefix, especially 'toatime'")
ARG("gt", "guard time in :math:`\mu s`")
ARG("fin", "fin message delay in :math:`\mu s`")
ARG("res1", "first response message delay in :math:`\mu s`")
ARG("res2", "second response message delay in :math:`\mu s`")
COMMENT("see :ref:`toatime`")

ADD_ITEM_M(1307, INF_CLEARED, "cleared f:%s")
ARG("f", "clear flags")
COMMENT("see :ref:`clear`")

ADD_ITEM_M(1310, INF_CLEAR_HELP, "clear [-m,-p,-mp]")
COMMENT("see :ref:`clear`")

ADD_ITEM_M(1311, INF_SETANCHORS_SET, "setanchors set cnt:%d anchors")
ARG("cnt:", "number of anchor in temporary table")
COMMENT("see :ref:`setanchors`")

ADD_ITEM_M(1312, INF_SETTAGS_SET, "settags set t:%d tags with a:%d anchors")
ARG("t", "number of tags")
ARG("t", "number of anchors")
COMMENT("see :ref:`setanchors`")
COMMENT("see :ref:`settags`")

ADD_ITEM_M(1313, INF_DELETETAGS, "deletetags deleted t:%d tags")
ARG("t", "number of deleted tag")
COMMENT("see :ref:`deletetags`")

// ranging TDOA
ADD_ITEM_M(1350, INF_TDOA_BEACON_FROM_TAG, "tdoa_tag did:%X anchor:%X mV:%d serial:%X%08X tsrg:%X%08X tsrl:%X%08X")
ARG("did", "tag short identification number")
ARG("mV", "tag battery voltage")
ARG("serial", "Tag serial number - this parameter may be deleted in a future")
ARG("tsrg", "TimeSample of Receive packet in Global time domain (after clock synchronization)")
ARG("tsrl", "TimeSample of Receive packet in Local time domain (without clock synchronization)")

ADD_ITEM_M(1351, INF_TDOA_BEACON_FROM_ANCHOR, "tdoa_anchor at:%X ar:%X tstg:%X%08X tstl:%X%08X tsrg:%X%08X tsrl:%X%08X tof:%d")
ARG("at", "identificator (DID) of anchor witch transmit beacon")
ARG("ar", "identificator (DID) of anchor witch received beacon")
ARG("tstg", "TimeSample of Transmission packet in Global time domain  in transmiting device (after clock synchronization)")
ARG("tstl", "TimeSample of Transmission packet in Local time domain in transmiting device (without clock synchronization)")
ARG("tsrg", "TimeSample of Receive packet in Global time domain in receiving device (after clock synchronization)")
ARG("tsrl", "TimeSample of Receive packet in Local time domain in receiving device (without clock synchronization)")

// settings
ADD_ITEM_M(1401, INF_SETTINGS_SAVED, "settings saved did:%X")
COMMENT("see :ref:`save`")

ADD_ITEM_M(1402, INF_SETTINGS_NO_CHANGES, "no changes to be saved did:%X")
COMMENT("see :ref:`save`")

// others
ADD_ITEM_M(1501, INF_IMU_SETTINGS, "imu delay:%d enable:%d did:%X")
ARG("delay", "imu delay before asleep when there is no motion")
ARG("enable", "when imu is enabled then tag go asleep after long time without motion {0-off, 1-on}")

ADD_ITEM_M(1502, INF_FU_SUCCESS, "Firmware upgrade success")
COMMENT("only from target device (during SINK upgrade)")

ADD_ITEM_M(1505, INF_LIST_SETTINGS, "listset [%s]")
ARG("%s", "list of settings command to save separated by ';'")
COMMENT("In list there will be omitted 'measure' an 'parent' commands, it should be realized separately")
COMMENT("List of settings commands is generated from sink device, because he need to know how to process them. "
		"It implies that list of commands of target device could be slightly different and here always will"
		"be a chance to loose some settings")

ADD_ITEM_M(1505, INF_LIST_SETTINGS_SINK, "listset [%s;%s]")
COMMENT("Work same as :ref:`INF_LIST_SETTINGS' but print values for anchor and for sink")
