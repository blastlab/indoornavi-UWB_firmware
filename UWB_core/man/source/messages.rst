.. _messages:

========
Messages
========

.. _information messages:

Informations
============

  .. _INF_DEVICE_TURN_ON:

**INF_DEVICE_TURN_ON**


*code:* 1101

*descriptor:* "Device turn on did:%X fV:%d"

  .. _INF_DEVICE_TURN_OFF:

**INF_DEVICE_TURN_OFF**


*code:* 1102

*descriptor:* "Device turn off did:%X"

  .. _INF_BEACON:

**INF_BEACON**


*code:* 1103

*descriptor:* "Beacon from did:%X"

  .. _INF_DEV_ACCEPTED:

**INF_DEV_ACCEPTED**


*code:* 1104

*descriptor:* "Device accepted, sink:%X parent:%X"

  .. _INF_PARENT_DESCRIPTION:

**INF_PARENT_DESCRIPTION**


*code:* 1106

*descriptor:* "parent of %X is %X (%d)"

  .. _INF_PARENT_SET:

**INF_PARENT_SET**


*code:* 1107

*descriptor:* "parent %X for %d devices, failed for %d"

  .. _INF_PARENT_CNT:

**INF_PARENT_CNT**


*code:* 1108

*descriptor:* "parent cnt:%d"

  .. _INF_STATUS:

**INF_STATUS**


*code:* 1111

*descriptor:* "stat did:%x mV:%d Rx:%d Tx:%d Er:%d To:%d Uptime:%dd.%dh.%dm.%ds"

  .. _INF_VERSION:

**INF_VERSION**


*code:* 1112

*descriptor:* "version %X %s %d.%d %d.%d.%X"

  .. _INF_ROUTE:

**INF_ROUTE**


*code:* 1113

*descriptor:* "route auto:%d"

  .. _INF_RF_SETTINGS:

**INF_RF_SETTINGS**


*code:* 1201

*descriptor:* "rfset ch:%d-%d/%d br:%d plen:%d prf:%d pac:%d code:%d nsSfd:%d sfdTo:%d"

  .. _INF_BLE_SETTINGS:

**INF_BLE_SETTINGS**


*code:* 1202

*descriptor:* "ble txpower:%d (-40/-20/-16/-12/-8/-4/0/3/4) enable:%d (0/1) did:%X"

  .. _INF_MEASURE_DATA:

**INF_MEASURE_DATA**


*code:* 1301

*descriptor:* "a %X>%X %d %d %d %d"

  .. _INF_MEASURE_INFO:

**INF_MEASURE_INFO**


*code:* 1302

*descriptor:* "measure %X with [%s]"

  .. _INF_MEASURE_CMD_CNT:

**INF_MEASURE_CMD_CNT**


*code:* 1303

*descriptor:* "measure cnt:%d"

  .. _INF_MEASURE_CMD_SET:

**INF_MEASURE_CMD_SET**


*code:* 1304

*descriptor:* "measure set %X with %d anchors"

  .. _INF_RANGING_TIME:

**INF_RANGING_TIME**


*code:* 1305

*descriptor:* "rangingtime T:%d t:%d (N:%d)"

  .. _INF_TOA_SETTINGS:

**INF_TOA_SETTINGS**


*code:* 1306

*descriptor:* "%s gt:%d fin:%d resp1:%d resp2:%d"

*comment*: prefix is dependent of usage

  .. _INF_CLEARED_PARENTS:

**INF_CLEARED_PARENTS**


*code:* 1307

*descriptor:* "cleared parents"

  .. _INF_CLEARED_MEASURES:

**INF_CLEARED_MEASURES**


*code:* 1308

*descriptor:* "cleared measures"

  .. _INF_CLEARED_PARENTS_AND_MEASURES:

**INF_CLEARED_PARENTS_AND_MEASURES**


*code:* 1309

*descriptor:* "cleared parents and measures"

  .. _INF_CLEAR_HELP:

**INF_CLEAR_HELP**


*code:* 1310

*descriptor:* "clear [-m,-p,-mp]"

  .. _INF_SETANCHORS_SET:

**INF_SETANCHORS_SET**


*code:* 1311

*descriptor:* "setanchors set %d anchors"

  .. _INF_SETTAGS_SET:

**INF_SETTAGS_SET**


*code:* 1312

*descriptor:* "settags set %d tags with %d anchors"

  .. _INF_DELETETAGS:

**INF_DELETETAGS**


*code:* 1313

*descriptor:* "deletetags deleted %d tags"

  .. _INF_SETTINGS_SAVED:

**INF_SETTINGS_SAVED**


*code:* 1401

*descriptor:* "settings saved did:%X"

  .. _INF_SETTINGS_NO_CHANGES:

**INF_SETTINGS_NO_CHANGES**


*code:* 1402

*descriptor:* "no changes to be saved did:%X"

  .. _INF_IMU_SETTINGS:

**INF_IMU_SETTINGS**


*code:* 1501

*descriptor:* "imu delay:%d enable:%d (0/1) did:%X"

  .. _INF_FU_SUCCESS:

**INF_FU_SUCCESS**


*code:* 1502

*descriptor:* "Firmware upgrade success"


.. _warning messages:

Warnings
============

  .. _WRN_CARRY_INCOMPATIBLE_VERSION:

**WRN_CARRY_INCOMPATIBLE_VERSION**


*code:* 1101

*descriptor:* "CARRY incompatible version %d (%d)"

  .. _WRN_CARRY_TARGET_NOBODY:

**WRN_CARRY_TARGET_NOBODY**


*code:* 1102

*descriptor:* "CARRY target nobody"

  .. _WRN_MAC_FRAME_BAD_OPCODE:

**WRN_MAC_FRAME_BAD_OPCODE**


*code:* 1103

*descriptor:* "MAC frame with bad opcode %X"

  .. _WRN_MAC_UNSUPPORTED_MAC_FRAME:

**WRN_MAC_UNSUPPORTED_MAC_FRAME**


*code:* 1104

*descriptor:* "MAC unsupported frame type %X"

  .. _WRN_MAC_UNSUPPORTED_ACK_FRAME:

**WRN_MAC_UNSUPPORTED_ACK_FRAME**


*code:* 1105

*descriptor:* "MAC ack frame is not supported yet"

  .. _WRN_FIRWARE_NOT_ACCEPTED_YET:

**WRN_FIRWARE_NOT_ACCEPTED_YET**


*code:* 1108

*descriptor:* "new firmware not accepted yet! did:%X"

  .. _WRN_SINK_ACCEPT_SINK:

**WRN_SINK_ACCEPT_SINK**


*code:* 1109

*descriptor:* "sink can't have any parent"

  .. _WRN_MAC_TX_ERROR:

**WRN_MAC_TX_ERROR**


*code:* 1201

*descriptor:* "Tx err"

  .. _WRN_MAC_TOO_BIG_FRAME:

**WRN_MAC_TOO_BIG_FRAME**


*code:* 1202

*descriptor:* "Frame with size %d can't be send within %dus slot"

  .. _WRN_RANGING_TOO_SMALL_PERIOD:

**WRN_RANGING_TOO_SMALL_PERIOD**


*code:* 1301

*descriptor:* "Too small period! Now N:%d T:%d"


.. _error messages:

Errors
============

  .. _WRN_CARRY_INCOMPATIBLE_VERSION:

**WRN_CARRY_INCOMPATIBLE_VERSION**


*code:* 1101

*descriptor:* "CARRY incompatible version %d (%d)"

  .. _WRN_CARRY_TARGET_NOBODY:

**WRN_CARRY_TARGET_NOBODY**


*code:* 1102

*descriptor:* "CARRY target nobody"

  .. _WRN_MAC_FRAME_BAD_OPCODE:

**WRN_MAC_FRAME_BAD_OPCODE**


*code:* 1103

*descriptor:* "MAC frame with bad opcode %X"

  .. _WRN_MAC_UNSUPPORTED_MAC_FRAME:

**WRN_MAC_UNSUPPORTED_MAC_FRAME**


*code:* 1104

*descriptor:* "MAC unsupported frame type %X"

  .. _WRN_MAC_UNSUPPORTED_ACK_FRAME:

**WRN_MAC_UNSUPPORTED_ACK_FRAME**


*code:* 1105

*descriptor:* "MAC ack frame is not supported yet"

  .. _WRN_FIRWARE_NOT_ACCEPTED_YET:

**WRN_FIRWARE_NOT_ACCEPTED_YET**


*code:* 1108

*descriptor:* "new firmware not accepted yet! did:%X"

  .. _WRN_SINK_ACCEPT_SINK:

**WRN_SINK_ACCEPT_SINK**


*code:* 1109

*descriptor:* "sink can't have any parent"

  .. _WRN_MAC_TX_ERROR:

**WRN_MAC_TX_ERROR**


*code:* 1201

*descriptor:* "Tx err"

  .. _WRN_MAC_TOO_BIG_FRAME:

**WRN_MAC_TOO_BIG_FRAME**


*code:* 1202

*descriptor:* "Frame with size %d can't be send within %dus slot"

  .. _WRN_RANGING_TOO_SMALL_PERIOD:

**WRN_RANGING_TOO_SMALL_PERIOD**


*code:* 1301

*descriptor:* "Too small period! Now N:%d T:%d"


.. critical messages:

Critical
============

  .. _CRIT_OTHER:

**CRIT_OTHER**


*code:* 1000

*descriptor:* "Critical error"

  .. _CRIT_LOG_CODES_ARE_NOT_UNIQ:

**CRIT_LOG_CODES_ARE_NOT_UNIQ**


*code:* 1001

*descriptor:* "logger codes aren't uniq, code:%d"

  .. _CRIT_LOG_CODES_ARE_NOT_MONOTONOUS:

**CRIT_LOG_CODES_ARE_NOT_MONOTONOUS**


*code:* 1002

*descriptor:* "logger codes aren't monotonous, code:%d"


.. _test messages:

Test
============

  .. _TEST_PASS:

**TEST_PASS**


*code:* 1000

*descriptor:* "PASS"

