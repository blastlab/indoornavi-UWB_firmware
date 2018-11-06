.. _messages:

================
Messages
================

.. _information messages:

Information
================

.. _INF_DEVICE_TURN_ON:

*INF_DEVICE_TURN_ON*
------------------------------------------------------------

*code:* 1101

*descriptor:* "Device turn on did:%X fV:%d"

 arg *fV*: 
	firmware major version

.. _INF_DEVICE_TURN_OFF:

*INF_DEVICE_TURN_OFF*
------------------------------------------------------------

*code:* 1102

*descriptor:* "Device turn off did:%X"

.. _INF_BEACON:

*INF_BEACON*
------------------------------------------------------------

*code:* 1103

*descriptor:* "Beacon from did:%X serial:%X%X mV:%d route:[%s]"

 arg *route*: 
	message route in format '[%X>%X>%X...] where last position is target address

.. _INF_DEV_ACCEPTED:

*INF_DEV_ACCEPTED*
------------------------------------------------------------

*code:* 1104

*descriptor:* "Device accepted, sink:%X parent:%X"

 arg *sink*: 
	hex address of parent sink

 arg *parent*: 
	hex address of parent device (in sink direction)

.. _INF_PARENT_DESCRIPTION:

*INF_PARENT_DESCRIPTION*
------------------------------------------------------------

*code:* 1106

*descriptor:* "parent of c:%X is p:%X # n:%d"

 arg *c*: 
	hex child did

 arg *p*: 
	hex parent did, parent for sink is always 0

 arg *n*: 
	number of child hops to sink

*comment*: see :ref:`parent`

.. _INF_PARENT_SET:

*INF_PARENT_SET*
------------------------------------------------------------

*code:* 1107

*descriptor:* "parent p:%X for s:%d devices, failed for f:%d"

 arg *p*: 
	hex parent did

 arg *s*: 
	number of devices, where parent has been set to a given one

 arg *f*: 
	number of devices, where parent has not been set to a given one

*comment*: see :ref:`parent`

.. _INF_PARENT_CNT:

*INF_PARENT_CNT*
------------------------------------------------------------

*code:* 1108

*descriptor:* "parent cnt:%d"

 arg *cnt*: 
	number of saved parent in volatile memory

*comment*: see :ref:`parent`

.. _INF_STATUS:

*INF_STATUS*
------------------------------------------------------------

*code:* 1111

*descriptor:* "stat did:%x mV:%d Rx:%d Tx:%d Er:%d To:%d Uptime:%dd.%dh.%dm.%ds"

 arg *mV*: 
	battery voltage in millivolts, correct values for devices with battery is 3100..4300, for devices without battery 4500..5200

 arg *Rx*: 
	12-bits counter of correctly received packets, values in range 0..4095. After overflow it will count from 0.

 arg *Tx*: 
	12-bits counter of correctly transmitted packets, values in range 0..4095. After overflow it will count from 0.

 arg *Er*: 
	12-bits counter of receiving packets error, values in range 0..4095. After overflow it will count from 0.

 arg *To*: 
	12-bits counter of timeout during transmitting or receiving frames, values in range 0..4095. After overflow it will count from 0.

 arg *Uptime*: 
	device work time in format days.hours.minuts.seconds. It overflow ofter 49.7 days.

*comment*: see :ref:`status`

.. _INF_VERSION:

*INF_VERSION*
------------------------------------------------------------

*code:* 1112

*descriptor:* "version did:%X serial:%X%X r:%s hV:%d.%d fV:%d.%d.%X%X"

 arg *serial*: 
	64-bit device unique identificator number

 arg *r*: 
	device role, possible values {SINK, ANCHOR, TAG, LISTENER, DEFAULT, OTHER}

 arg *hV*: 
	hardware version, major.minor

 arg *fV*: 
	formware version major.minor.hash where source repository commit hash is in hexadecimal and is 32-bit value.

*comment*: see :ref:`version`

.. _INF_ROUTE:

*INF_ROUTE*
------------------------------------------------------------

*code:* 1113

*descriptor:* "route auto:%d"

 arg *auto*: 
	automaticaly route module status {0-off, 1-on}

*comment*: see :ref:`route`

.. _INF_MAC:

*INF_MAC*
------------------------------------------------------------

*code:* 1114

*descriptor:* "mac did:%X pan:%X beacon:%d sp:%d st:%d gt:%d raad:%d role:%s"

 arg *pan*: 
	personal area network identifier

 arg *beacon*: 
	interval in :math:`ms`

 arg *sp*: 
	slot period in :math:`\mu s`

 arg *st*: 
	one slot time in :math:`\mu s`

 arg *gt*: 
	slot guard time in :math:`\mu s`

 arg *raad*: 
	raport anchor to anchor distances boolean

 arg *role*: 
	device role, possible values {SINK, ANCHOR, TAG, LISTENER, DEFAULT, OTHER}

.. _INF_RF_SETTINGS:

*INF_RF_SETTINGS*
------------------------------------------------------------

*code:* 1201

*descriptor:* "rfset ch:%d-%d/%d br:%d plen:%d prf:%d pac:%d code:%d nsSfd:%d sfdTo:%d smartTx:%d"

 arg *ch*: 
	channel number - (frequency/bandwidth

 arg *br*: 
	baudrate in kbps

 arg *plen*: 
	preamble length

 arg *prf*: 
	pulse repetition frequency in MHz

 arg *pac*: 
	preamble acquisition chunk size

 arg *code*: 
	communication code

 arg *nsSfd*: 
	non standard frame delimiter {0-off, 1-on}

 arg *sfdTo*: 
	SFD detection timeout count

 arg *smartTx*: 
	smart tx booster for short messages {0-off, 1-on}

*comment*: see :ref:`rfset`

.. _INF_RF_TX_SETTINGS:

*INF_RF_TX_SETTINGS*
------------------------------------------------------------

*code:* 1202

*descriptor:* "txset did:%X pgdly:%d P1:%d+%d.%d P2:%d+%d.%d P3:%d+%d.%d P4:%d+%d.%d"

 arg *pgdly*: 
	power generator delay

 arg *P1*: 
	power gain in db for shoertest messages (<0.125ms)

 arg *P2*: 
	power gain in db for short messages (<0.25ms)

 arg *P3*: 
	power gain in db for long messages (<0.5ms

 arg *P4*: 
	power gain in db for longest mesages (>=0.5ms)

*comment*: In smart tx power is disabled, then only P4 is used

.. _INF_BLE_SETTINGS:

*INF_BLE_SETTINGS*
------------------------------------------------------------

*code:* 1203

*descriptor:* "ble txpower:%d (-40/-20/-16/-12/-8/-4/0/3/4) enable:%d (0/1) did:%X"

 arg *txpower*: 
	ble transmitter power settings

 arg *enable*: 
	bluetooth module status

*comment*: see :ref:`ble`

.. _INF_MEASURE_DATA:

*INF_MEASURE_DATA*
------------------------------------------------------------

*code:* 1301

*descriptor:* "a %X>%X %d %d %d %d"

 arg *first*: 
	hex did of first device

 arg *second*: 
	hex did of second device

 arg *third*: 
	distance in cm

 arg *fourth*: 
	RSSI in dBm*100

 arg *fifth*: 
	FPP in dBm*100

*comment*: see :ref:`measure`

.. _INF_MEASURE_INFO:

*INF_MEASURE_INFO*
------------------------------------------------------------

*code:* 1302

*descriptor:* "measure t:%X with a:[%s]"

 arg *t*: 
	hex target device address

 arg *a*: 
	list of hex anchors addresses in one measure

*comment*: see :ref:`measure`

.. _INF_MEASURE_CMD_CNT:

*INF_MEASURE_CMD_CNT*
------------------------------------------------------------

*code:* 1303

*descriptor:* "measure cnt:%d"

 arg *cnt*: 
	measure counter in volatile memory

*comment*: see :ref:`measure`

.. _INF_MEASURE_CMD_SET:

*INF_MEASURE_CMD_SET*
------------------------------------------------------------

*code:* 1304

*descriptor:* "measure set t:%X with cnt:%d anchors"

 arg *t*: 
	hex target device address

 arg *cnt*: 
	number of new measures

*comment*: see :ref:`measure`

.. _INF_RANGING_TIME:

*INF_RANGING_TIME*
------------------------------------------------------------

*code:* 1305

*descriptor:* "rangingtime T:%d t:%d (N:%d)"

 arg *T*: 
	ranging period

 arg *t*: 
	delay between ranging

 arg *N*: 
	number of ranging slot in a given period

*comment*: see :ref:`rangingtime`

.. _INF_TOA_SETTINGS:

*INF_TOA_SETTINGS*
------------------------------------------------------------

*code:* 1306

*descriptor:* "%s gt:%d fin:%d resp1:%d resp2:%d"

 arg *first*: 
	usage dependant prefix, especially 'toatime'

 arg *gt*: 
	guard time in :math:`\mu s`

 arg *fin*: 
	fin message delay in :math:`\mu s`

 arg *res1*: 
	first response message delay in :math:`\mu s`

 arg *res2*: 
	second response message delay in :math:`\mu s`

*comment*: see :ref:`toatime`

.. _INF_CLEARED:

*INF_CLEARED*
------------------------------------------------------------

*code:* 1307

*descriptor:* "cleared f:%s"

 arg *f*: 
	clear flags

*comment*: see :ref:`clear`

.. _INF_CLEAR_HELP:

*INF_CLEAR_HELP*
------------------------------------------------------------

*code:* 1310

*descriptor:* "clear [-m,-p,-mp]"

*comment*: see :ref:`clear`

.. _INF_SETANCHORS_SET:

*INF_SETANCHORS_SET*
------------------------------------------------------------

*code:* 1311

*descriptor:* "setanchors set cnt:%d anchors"

 arg *cnt:*: 
	number of anchor in temporary table

*comment*: see :ref:`setanchors`

.. _INF_SETTAGS_SET:

*INF_SETTAGS_SET*
------------------------------------------------------------

*code:* 1312

*descriptor:* "settags set t:%d tags with a:%d anchors"

 arg *t*: 
	number of tags

 arg *t*: 
	number of anchors

*comment*: see :ref:`setanchors`

*comment*: see :ref:`settags`

.. _INF_DELETETAGS:

*INF_DELETETAGS*
------------------------------------------------------------

*code:* 1313

*descriptor:* "deletetags deleted t:%d tags"

 arg *t*: 
	number of deleted tag

*comment*: see :ref:`deletetags`

.. _INF_SETTINGS_SAVED:

*INF_SETTINGS_SAVED*
------------------------------------------------------------

*code:* 1401

*descriptor:* "settings saved did:%X"

*comment*: see :ref:`save`

.. _INF_SETTINGS_NO_CHANGES:

*INF_SETTINGS_NO_CHANGES*
------------------------------------------------------------

*code:* 1402

*descriptor:* "no changes to be saved did:%X"

*comment*: see :ref:`save`

.. _INF_IMU_SETTINGS:

*INF_IMU_SETTINGS*
------------------------------------------------------------

*code:* 1501

*descriptor:* "imu delay:%d enable:%d did:%X"

 arg *delay*: 
	imu delay before asleep when there is no motion

 arg *enable*: 
	when imu is enabled then tag go asleep after long time without motion {0-off, 1-on}

.. _INF_FU_SUCCESS:

*INF_FU_SUCCESS*
------------------------------------------------------------

*code:* 1502

*descriptor:* "Firmware upgrade success"

*comment*: only from target device (during SINK upgrade)


.. _warning messages:

Warnings
================

.. _WRN_CARRY_INCOMPATIBLE_VERSION:

*WRN_CARRY_INCOMPATIBLE_VERSION*
------------------------------------------------------------

*code:* 1101

*descriptor:* "CARRY incompatible version %d (%d)"

.. _WRN_CARRY_TARGET_NOBODY:

*WRN_CARRY_TARGET_NOBODY*
------------------------------------------------------------

*code:* 1102

*descriptor:* "CARRY target nobody"

.. _WRN_MAC_FRAME_BAD_OPCODE:

*WRN_MAC_FRAME_BAD_OPCODE*
------------------------------------------------------------

*code:* 1103

*descriptor:* "MAC frame with bad opcode %X"

.. _WRN_MAC_UNSUPPORTED_MAC_FRAME:

*WRN_MAC_UNSUPPORTED_MAC_FRAME*
------------------------------------------------------------

*code:* 1104

*descriptor:* "MAC unsupported frame type %X"

.. _WRN_MAC_UNSUPPORTED_ACK_FRAME:

*WRN_MAC_UNSUPPORTED_ACK_FRAME*
------------------------------------------------------------

*code:* 1105

*descriptor:* "MAC ack frame is not supported yet"

.. _WRN_FIRWARE_NOT_ACCEPTED_YET:

*WRN_FIRWARE_NOT_ACCEPTED_YET*
------------------------------------------------------------

*code:* 1108

*descriptor:* "new firmware not accepted yet! did:%X"

.. _WRN_SINK_ACCEPT_SINK:

*WRN_SINK_ACCEPT_SINK*
------------------------------------------------------------

*code:* 1109

*descriptor:* "sink can't have any parent"

.. _WRN_CARRY_TOO_MUCH_TAGS_TO_TRACK:

*WRN_CARRY_TOO_MUCH_TAGS_TO_TRACK*
------------------------------------------------------------

*code:* 1110

*descriptor:* "there is too much tags to track (max:%d)"

.. _WRN_MAC_TX_ERROR:

*WRN_MAC_TX_ERROR*
------------------------------------------------------------

*code:* 1201

*descriptor:* "Tx err"

.. _WRN_MAC_TOO_BIG_FRAME:

*WRN_MAC_TOO_BIG_FRAME*
------------------------------------------------------------

*code:* 1202

*descriptor:* "Frame with size %d can't be send within %dus slot"

.. _WRN_RANGING_TOO_SMALL_PERIOD:

*WRN_RANGING_TOO_SMALL_PERIOD*
------------------------------------------------------------

*code:* 1301

*descriptor:* "Too small period! Now N:%d T:%d"


.. _error messages:

Errors
================

.. _ERR_MAC_NO_MORE_BUFFERS:

*ERR_MAC_NO_MORE_BUFFERS*
------------------------------------------------------------

*code:* 1101

*descriptor:* "No more buffers"

.. _ERR_MAC_BAD_OPCODE_LEN:

*ERR_MAC_BAD_OPCODE_LEN*
------------------------------------------------------------

*code:* 1102

*descriptor:* "%s bad len %d!=%d"

 arg *%s*: 
	function code name

 arg *%d*: 
	received length

 arg *%d*: 
	expected length

.. _ERR_BAD_OPCODE_LEN:

*ERR_BAD_OPCODE_LEN*
------------------------------------------------------------

*code:* 1103

*descriptor:* "%s bad len %d!=%d"

 arg *%s*: 
	function code name

 arg *%d*: 
	received length

 arg *%d*: 
	expected length

.. _ERR_BAD_OPCODE:

*ERR_BAD_OPCODE*
------------------------------------------------------------

*code:* 1104

*descriptor:* "unknown opcode %Xh"

.. _ERR_PARENT_FOR_SINK:

*ERR_PARENT_FOR_SINK*
------------------------------------------------------------

*code:* 1105

*descriptor:* "parent can't be set for sink"

.. _ERR_PARENT_NEED_ANCHOR:

*ERR_PARENT_NEED_ANCHOR*
------------------------------------------------------------

*code:* 1106

*descriptor:* "parent must be an anchor (%X)"

 arg *%X*: 
	address of incorrect device

.. _ERR_BEACON_TOO_MANY_HOPS:

*ERR_BEACON_TOO_MANY_HOPS*
------------------------------------------------------------

*code:* 1107

*descriptor:* "beacon make too many hops (%d)"

 arg *%d*: 
	maximum number of beaacon hops

.. _ERR_RF_BAD_CHANNEL:

*ERR_RF_BAD_CHANNEL*
------------------------------------------------------------

*code:* 1201

*descriptor:* "rfset ch 1..7 (without 6)"

.. _ERR_RF_BAD_BAUDRATE:

*ERR_RF_BAD_BAUDRATE*
------------------------------------------------------------

*code:* 1202

*descriptor:* "rfset br 110/850/6800"

.. _ERR_RF_BAD_PREAMBLE_LEN:

*ERR_RF_BAD_PREAMBLE_LEN*
------------------------------------------------------------

*code:* 1203

*descriptor:* "rfset plen 64/128/256/512/1024/1536/2048/4096"

.. _ERR_RF_BAD_PRF:

*ERR_RF_BAD_PRF*
------------------------------------------------------------

*code:* 1204

*descriptor:* "rfset prf 16/64"

.. _ERR_RF_BAD_PAC:

*ERR_RF_BAD_PAC*
------------------------------------------------------------

*code:* 1205

*descriptor:* "rfset pac 8/16/32/64"

.. _ERR_RF_BAD_CODE:

*ERR_RF_BAD_CODE*
------------------------------------------------------------

*code:* 1206

*descriptor:* "rfset code 1..24"

.. _ERR_RF_BAD_NSSFD:

*ERR_RF_BAD_NSSFD*
------------------------------------------------------------

*code:* 1207

*descriptor:* "rfset nssfd 0/1"

.. _ERR_RF_TX_NEED_COARSE_AND_FINE_P:

*ERR_RF_TX_NEED_COARSE_AND_FINE_P*
------------------------------------------------------------

*code:* 1208

*descriptor:* "txset need P%dc and P%df at the same time"

 arg *%d*: 
	number of P argument

 arg *%d*: 
	number of P argument

.. _ERR_RF_TX_BAD_COARSE_P:

*ERR_RF_TX_BAD_COARSE_P*
------------------------------------------------------------

*code:* 1209

*descriptor:* "txset P%dc must be divisible by 3 and <=18"

 arg *%d*: 
	number of P argument

.. _ERR_RF_TX_BAD_FINE_P:

*ERR_RF_TX_BAD_FINE_P*
------------------------------------------------------------

*code:* 1210

*descriptor:* "txset P%df must be <=31"

 arg *%d*: 
	number of P argument

.. _ERR_BLE_INACTIVE:

*ERR_BLE_INACTIVE*
------------------------------------------------------------

*code:* 1211

*descriptor:* "BLE is disabled"

*comment*: BLE module is not included into this version of firmware

.. _ERR_BLE_BAD_TXPOWER:

*ERR_BLE_BAD_TXPOWER*
------------------------------------------------------------

*code:* 1212

*descriptor:* "Wrong ble txpower value"

*comment*: BLE module is not included into this version of firmware

.. _ERR_MEASURE_ADD_ANCHOR_FAILED_DID:

*ERR_MEASURE_ADD_ANCHOR_FAILED_DID*
------------------------------------------------------------

*code:* 1301

*descriptor:* "measure add anchor failed with %X"

 arg *hex*: 
	incorrect anchor address

.. _ERR_MEASURE_TARGET_WITH_ANC_FAILED:

*ERR_MEASURE_TARGET_WITH_ANC_FAILED*
------------------------------------------------------------

*code:* 1302

*descriptor:* "measure target failed ancCnt:%d"

 arg *ancCnt*: 
	number of anchors to connect with target

.. _ERR_SETANCHORS_FAILED:

*ERR_SETANCHORS_FAILED*
------------------------------------------------------------

*code:* 1303

*descriptor:* "setanchors failed (%X)"

 arg *hex*: 
	address of device which cause error

.. _ERR_SETTAGS_NEED_SETANCHORS:

*ERR_SETTAGS_NEED_SETANCHORS*
------------------------------------------------------------

*code:* 1304

*descriptor:* "settags need setanchors"

.. _ERR_SETTAGS_FAILED:

*ERR_SETTAGS_FAILED*
------------------------------------------------------------

*code:* 1305

*descriptor:* "settags failed after %X"

 arg *hex*: 
	address of device which cause error

.. _ERR_MAC_RAAD_BAD_VALUE:

*ERR_MAC_RAAD_BAD_VALUE*
------------------------------------------------------------

*code:* 1306

*descriptor:* "mac raad value must be 0 or 1 (enable)"

.. _ERR_MAC_ADDR_BAD_VALUE:

*ERR_MAC_ADDR_BAD_VALUE*
------------------------------------------------------------

*code:* 1307

*descriptor:* "mac addr bad value"

.. _ERR_MAC_BEACON_TIMER_PERIOD_TOO_SHORT:

*ERR_MAC_BEACON_TIMER_PERIOD_TOO_SHORT*
------------------------------------------------------------

*code:* 1308

*descriptor:* "mac beacon period must be greater than %d"

 arg *%d*: 
	minumum beacon period value

.. _ERR_FLASH_ERASING:

*ERR_FLASH_ERASING*
------------------------------------------------------------

*code:* 1401

*descriptor:* "flash erasing error did:%X"

.. _ERR_FLASH_WRITING:

*ERR_FLASH_WRITING*
------------------------------------------------------------

*code:* 1402

*descriptor:* "flash writing error did:%X"

.. _ERR_FLASH_OTHER:

*ERR_FLASH_OTHER*
------------------------------------------------------------

*code:* 1403

*descriptor:* "SETTINGS_Save bad implementation did:%X"

.. _ERR_BAD_COMMAND:

*ERR_BAD_COMMAND*
------------------------------------------------------------

*code:* 1501

*descriptor:* "Bad command"

.. _ERR_BASE64_TOO_LONG_INPUT:

*ERR_BASE64_TOO_LONG_INPUT*
------------------------------------------------------------

*code:* 1502

*descriptor:* "TXT_Bin too long base64 message"

.. _ERR_BASE64_TOO_LONG_OUTPUT:

*ERR_BASE64_TOO_LONG_OUTPUT*
------------------------------------------------------------

*code:* 1503

*descriptor:* "LOG_Bin too long base64 message, FC:%xh"

 arg *FC*: 
	hexadecimal function code which cause error


.. _critical messages:

Critical
================

.. _CRIT_OTHER:

*CRIT_OTHER*
------------------------------------------------------------

*code:* 1000

*descriptor:* "Critical error"

.. _CRIT_LOG_CODES_ARE_NOT_UNIQ:

*CRIT_LOG_CODES_ARE_NOT_UNIQ*
------------------------------------------------------------

*code:* 1001

*descriptor:* "logger codes aren't uniq, code:%d"

 arg *code*: 
	message code

*comment*: it is logger self test error

.. _CRIT_LOG_CODES_ARE_NOT_MONOTONOUS:

*CRIT_LOG_CODES_ARE_NOT_MONOTONOUS*
------------------------------------------------------------

*code:* 1002

*descriptor:* "logger codes aren't monotonous, code:%d"

 arg *code*: 
	message code

*comment*: it is logger self test error

*comment*: when codes aren't monotonous then probability of error is bigger


.. _test messages:

Test
================

.. _TEST_PASS:

*TEST_PASS*
------------------------------------------------------------

*code:* 1000

*descriptor:* "PASS"

