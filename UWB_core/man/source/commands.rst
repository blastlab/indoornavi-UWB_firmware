.. _commands:

=========
Commands
=========

.. automodule: commands
    :members:
    :undoc-members:
    :show-inheritance:

General rules
=============

* parser is letter size independent during parsing commands and hexadecimal values
* when parameters has names then order is not important
* spaces between attribut name and values will be ommited



.. _status:

status
=========

*stat [did:hex]*

did:  [0..FFFE]  hexadecimal address of device to check

Get device status. Response is in format::

    stat did:hex mV:dec Rx:dec Tx:dec Er:dec To:dec Uptime:decd.dech.decm.decs

+-----------+-------------------------------------------------------------------------------------------------------------------------------------+
| *mV:*     | battery volatge in millivolts, correct values for devices with battery is 3100..4300, for devices without battery 4500..5200        |
+-----------+-------------------------------------------------------------------------------------------------------------------------------------+
| *Rx:*     | 12-bits counter of correctly received packets, values in range 0..4095. Aftetr overflow it will count from 0.                       |
+-----------+-------------------------------------------------------------------------------------------------------------------------------------+
| *Tx:*     | 12-bits counter of correctly transmitted packets, values in range 0..4095. Aftetr overflow it will count from 0.                    |
+-----------+-------------------------------------------------------------------------------------------------------------------------------------+
| *Ex:*     | 12-bits counter of receiving packets error, values in range 0..4095. Aftetr overflow it will count from 0.                          |
+-----------+-------------------------------------------------------------------------------------------------------------------------------------+
| *Tx:*     |  12-bits counter of timeout during transmitting or receiving frames, values in range 0..4095. Aftetr overflow it will count from 0. |
+-----------+-------------------------------------------------------------------------------------------------------------------------------------+
| *Uptime:* |  device work time in format days.hours.minuts.seconds. It overflow ofter 49.7 days.                                                 |
+-----------+-------------------------------------------------------------------------------------------------------------------------------------+

Example::

  > stat did:A151
  > stat did:A151 mV:3872 Rx:1123 Tx:1043 Er:25 To:1 Uptime:0d.14h.23m.15s



.. _version:

version
=======

*ver [did:hex]*

* *did:* [0..FFFE] hexadecimal address of device to check


Get device version and role description. Response is in format::


    version did:hex r:string hV:dec.dec fV:dec.dec.hex


* r: device role, possible values {*SINK*, *ANCHOR*, *TAG*, *LISTENER*, *DEFAULT*, *OTHER*}
* hV: hardware version, *major.minor*
* fV: formware version *major.minor.hash* where source repository commit hash is in hexadecimal and is 32-bit value.

::

    > version
    > version did:8012 r:SINK hV:1.2 fV:0.2.cb016c11



.. _rfset:

rfset
========

*rfset [ch:dec] [br:dec] [plen:dec] [prf:dec] [pac:dec] [code:dec] [sfdto:dec] [pgdly:dec] [nssfd:dec] [power:hex] [did:hex]*

* ch: radio channel. Change radio frequency and bandwidth. Possible values {1, 2, 3, 4, 5, 7}
* br: radio baudrate in kbps. Possible values {110, 850, 6800}
* plen: preamble length. Possible values {64, 128, 256, 512, 1024, 1546, 2048, 4096}
* prf: pulse repetition frequency in MHz. Possible values {16, 64}
* pac: preamble acquisition chunk. Possible values {8, 16, 32, 64}. See `recommended PAC size`_
* code: communication code. Possible values {1..24}
* sfdto: SFD detection timeout count. Should be bigger than plen+1 and lower than 65536.
* pgdly: pulse generator delay. It is used to adjust RF bandwidth. Possible values {1..255}. . See `recommended PG delay`_
* nssfd: non standard frame delimiter. Possible values {0, 1}
* power: transmitter power. This value consist of four 8-bits value.
  Each byte lowest 5 bits is in a 0.5dB gain resolution and highest 3 bits are in 3 dB resolution.
  Highest byte says about power of frame below 125 us duration, then below 250 us, 500 us and lowest byte is for longer frames.
* did: [0..FFFE] hexadecimal address of device to check

For more detailed description see `transceiver user manual <https://www.decawave.com/wp-content/uploads/2018/09/dw100020user20manual_0.pdf>`_

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

note: Each value should be same in each device during communication.
It it possible to loose radio connection after changing radio settings.
Values in each device should be same.


.. _save:

save
=========

* save [did:hex]*

* did: [0..FFFE] hexadecimal address of device to check

Save current settings in non-volatile memory.
In sink device it saves also current measure traces and parent settings.
As a result there should be 