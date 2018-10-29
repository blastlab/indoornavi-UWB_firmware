.. _commands:

=========
Commands
=========

.. automodule: commands
    :members:
    :undoc-members:
    :show-inheritance:

General rules
===============

* parser is letter size independent during parsing commands and hexadecimal values
* when parameters has names then order is not important
* spaces between attribute name and values will be omitted



.. _status:

status
===============

*stat [did:hex]*

did:  [0..FFFE]  hexadecimal address of device to check

Get device status. Response is in :ref:`INF_STATUS` format.

Example::

  > stat did:A151
  > stat did:A151 mV:3872 Rx:1123 Tx:1043 Er:25 To:1 Uptime:0d.14h.23m.15s



.. _version:

version
===============

*ver [did:hex]*

* *did:* [0..FFFE] hexadecimal address of device to check


Get device version and role description. Response is in :ref:`INF_VERSION` format.::

    > version
    > version did:8012 serial:ABC132456DE r:SINK hV:1.2 fV:0.2.cb016c11



.. _rfset:

rfset
===============

*rfset [ch:dec] [br:dec] [plen:dec] [prf:dec] [pac:dec] [code:dec] [sfdto:dec] [pgdly:dec] [nssfd:dec] [power:hex] [did:hex]*

* ch: radio channel. Change radio frequency and bandwidth. Possible values {1, 2, 3, 4, 5, 7}
* br: radio baud rate in kbps. Possible values {110, 850, 6800}
* plen: preamble length. Possible values {64, 128, 256, 512, 1024, 1546, 2048, 4096}
* prf: pulse repetition frequency in MHz. Possible values {16, 64}
* pac: preamble acquisition chunk. Possible values {8, 16, 32, 64}. See `recommended PAC size`_
* code: communication code. Possible values {1..24}
* sfdto: SFD detection timeout count. Should be bigger than plen+1 and lower than 65536.
* nssfd: non standard frame delimiter. Possible values {0, 1}
* did: [0..FFFE] hexadecimal address of device to check

For more detailed description see `transceiver user manual <https://www.decawave.com/wp-content/uploads/2018/09/dw100020user20manual_0.pdf>`_

Response is in :ref:`INF_RF_SETTINGS` format.

Errors:

* :ref:`ERR_RF_BAD_PAC`
* :ref:`ERR_RF_BAD_CODE`
* :ref:`ERR_RF_BAD_NSSFD`
* :ref:`ERR_RF_BAD_PRF`
* :ref:`ERR_RF_BAD_PREAMBLE_LEN`
* :ref:`ERR_RF_BAD_BAUDRATE`
* :ref:`ERR_RF_BAD_CHANNEL`

.. _recommended PAC size:

recommended PAC size:

+------+-----------------+
| plen | recommended PAC |
+======+=================+
| 64   |     8           |
+------+-----------------+
| 128  |     8           |
+------+-----------------+
| 256  |     16          |
+------+-----------------+
| 512  |     32          |
+------+-----------------+
| 1024 |     64          |
+------+-----------------+
| 1536 |     64          |
+------+-----------------+
| 2048 |     64          |
+------+-----------------+
| 4096 |     64          |
+------+-----------------+

note: Each value should be same in each device during communication.
It it possible to loose radio connection after changing radio settings.
Values in each device should be same.

note: This command is only for advanced users


.. _txset:

txset
===============

*txset [pgdly:dec] [power:hex] [P1c:dec P1f:dec] [P2c:dec P2f:dec] [P3c:dec P3f:dec] [P4c:dec P4f:dec] [did:hex]*

* pgdly: pulse generator delay. It is used to adjust RF bandwidth. Possible values {1..255}. . See `recommended PG delay`_
* power: transmitter power. This value consist of four 8-bits value.
  Each byte lowest 5 bits is in a 0.5dB gain resolution and highest 3 bits are in 3 dB resolution.
  Highest byte says about power of frame below 125 us duration, then below 250 us, 500 us and lowest byte is for longer frames.
  for more information see `transceiver user manual`_ 7.2.31.3
* Pnc: transmitter coarse gain in dB, must be divisible by 3 and be lower or equal to 18
* Pnf: transmitter fine gain in 0.5dB units, must be lower or equal to 31 (15.5dB)

note: when smart tx power is disabled then always P4 is used

note: This command is only for advanced users

.. _recommended PG delay:

recommended PG delay:

=======  =======
Channel  pgdly
=======  =======
1        C9h
2        C2h
3        C5h
4        95h
5        C0h
7        93h
=======  =======



.. _save:

save
===============

*save [did:hex]*

* did: [0..FFFE] hexadecimal address of a target device

Save current settings in non-volatile memory.
In sink device it saves also current measure traces and parent settings.
As a result there should be 

Response is in :ref:`INF_SETTINGS_SAVED` or :ref:`INF_SETTINGS_NO_CHANGES` format.

Errors:

* :ref:`ERR_FLASH_ERASING`
* :ref:`ERR_FLASH_WRITING`
* :ref:`ERR_FLASH_OTHER`


.. _clear:

clear
===============

*clear [-m] [-p] [-mp] [did:hex]*

* -m clear measure table
* -p clear parent table
* -mp combination -m and -p, response :ref:`INF_CLEARED`
* did: [0..FFFE] hexadecimal address of a target device

Response is in :ref:`INF_CLEARED`
Clear values from settings, **only volatile memory**. To preserve changes use save_ command.

Errors:

* :ref:`INF_CLEAR_HELP`


.. _reset:

reset
=======

*reset [did:hex]*

Reset device.
After device turn on then :ref:`INF_DEVICE_TURN_ON` message should be received.


.. _bin:

bin
===============

*bin base64*

* convert base64 string to binary data and call binary parser.

Errors:

* :ref:`ERR_BASE64_TOO_LONG_INPUT`
* :ref:`ERR_BASE64_TOO_LONG_OUTPUT`
* :ref:`ERR_BAD_OPCODE`
* :ref:`ERR_BAD_OPCODE_LEN`


.. _setanchors:

setanchors
===============

*setanchors hex [,hex..]*

* list of anchors addresses

Fill temporary anchors table. This table is used especially in :ref:`settags` command.
Response is in :ref:`INF_SETANCHORS_SET` format.
Errors:
* :ref:`ERR_SETANCHORS_FAILED`


.. _settags:

settags
===============

*settags hex [hex..]*

* list of tags addresses

Add new items to measures init table in volatile memory.
There will be measure between each tag from list and anchor from temporary anchors list (see setanchors_).
Each measure will be singular - one measure in one slot.
To create nonsingular measures use measure_.
To preserve changes use save_.

Response will be in :ref:`INF_SETTAGS_SET`
Errors:

* :ref:`ERR_SETTAGS_NEED_SETANCHORS`
* :ref:`ERR_SETTAGS_FAILED`

Warnings:

* :ref:`WRN_RANGING_TOO_SMALL_PERIOD`


.. _measure:

measure
===============

*measure*

Response will be :ref:`INF_MEASURE_CMD_CNT`

*measure FFFF*

Response will be :ref:`INF_MEASURE_INFO`.
After each call measure read index will be incremented, so it is designed to scan whole measures init table.

*measure hex hex [hex..]*

* target device address
* list of anchor addresses to measure with target

Response will be :ref:`INF_MEASURE_CMD_SET`.

Errors:

* :ref:`ERR_MEASURE_ADD_ANCHOR_FAILED_DID`
* :ref:`ERR_MEASURE_TARGET_WITH_ANC_FAILED`

Warnings:

* :ref:`WRN_RANGING_TOO_SMALL_PERIOD`


.. _deletetags:

deletetags
===============

*deletetags hex [hex..]*

* list of measure targets to delete

Delete each item from measure init table where target is one of a given addresses

Response is :ref:`INF_DELETETAGS`


.. _rangingtime:

rangingtime
===============

*rangingtime*

Response is :ref:`INF_RANGING_TIME`.

*rangingtime [T:dec] [t:dec] [N:dec]*

* T: ranging period in :math:`ms`
* t: ranging time one slot time in :math:`ms`
* N: number of ranging slot in one period

When each parameter is specified then *N* will be ignored

Warnings:

* :ref:`WRN_RANGING_TOO_SMALL_PERIOD`


.. _toatime:

toatime
===============

*toatime*

Response is :ref:`INF_TOA_SETTINGS`

*toatime [gt:dec] [fin:dec] [resp?:dec]*

* gt: guard time in :math:`\mu s`
* fin: *final* message delay in :math:`\mu s`
  Time between transmission last *response* and *final* message.
* resp?: *respnse* delay in :math:`\mu s`. Replace *?* with *response* number.
  Time between receiving *poll* message and sending *reponse*.

note: Guard time change will fully affect after reset

note: This command is only for advanced users


.. _parent:

parent
===============

*parent*

Response is :ref:`INF_PARENT_CNT`.

*parent hex*

* address of asked device

Check anchor parent saved in sink volatile memory.

Response is :ref:`INF_PARENT_DESCRIPTION`

*parent hex hex [hex..]*

Result is :ref:`INF_PARENT_SET`

Errors:

* :ref:`ERR_PARENT_NEED_ANCHOR`
* :ref:`ERR_PARENT_FOR_SINK`


.. _ble:

ble
===============

*ble [did:hex]*

Response is :ref:`INF_BLE_SETTINGS`

*ble [txpower:dec] [enable:dec] [did:hex]*

* txpower is transmitter power in dBm {-40, -20, -16, -12, -8, -4, 0, 3, 4}
* enable advertisement {0-off, 1-on}

Response is :ref:`INF_BLE_SETTINGS`


.. _imu:

imu
===============

*imu [did:hex]*

Response is :ref:`INF_IMU_SETTINGS`.

*imu [delay:dec] [enable:dec] [did:hex]*

* delay in second before asleep in motionless state. Must be greater tah :math:`10s`.
* enable motionless asleep {0-off, 1-on}

Response is :ref:`INF_IMU_SETTINGS`.


.. _route:

route
===============

*route [auto:dec]*

* auto enable auto route builder module {0-off, 1-on}

Auto route module base on :ref:`INF_BEACON` messages.

Response is :ref:`INF_ROUTE`


.. _role:

role
===============

*role string*

* string possible values {sink, anchor, tag, listener}

Response is :ref:`INF_VERSION`.

note: To fully affect, save_ and reset_ may be needed.

note: this command is for debug purpose only


.. _hang:

hang
===============

*hang*

Go to infinity loop.

note: this command is for debug purpose only


.. _mac:

mac
===============

*mac [beacon:dec] [sp:dec] [st:dec] [gt:dec] [pan:hex] [addr:hex] [raad:dec] [-sink|-anchor|-tag|-default|-listener]*

* beacon timer interval in :math:`ms`
* sp is *slot period* time in :math:`\mu s`, must be greater than *st*
* st is one *slot time* in :math:`\mu s`, must be lower than *sp*
* gt is slot *guard time* in :math:`\mu s`
* pan is new device *personal area network* identifier, after change there might be a trouble with communication
* addr is new device address
* raad is *report anchor to anchor distances* boolean {0-off, 1-on}

Response is :ref:`INF_MAC`.

note: *beacon* and *raad* are allowed for each user, but other parameters must be changes carefully - only for advanced users
