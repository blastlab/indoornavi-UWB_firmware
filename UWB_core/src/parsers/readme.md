# Parsers structure

## Text commands parsing

Text commands are registered into __txt_parser.c__ internal buffer via __TXT_Input()__ function. Command can be inputted by parts. Each command ends with _'\n'_ and _'\r'_ are ignored. To execute commands from internal buffer __TXT_Control()__ function should be called - suggested is call from main program loop. This function call __TXT_Parse()__, look for _'did:'_ parameter, compare command with _txt_cb_tab_ array content and call callback from __txt_parser_cb.c__ file. Each callback should have _'TXT_'_ prefix, _'Cb' suffix and read values from input buffer, build corresponding frame structure and call __BIN_ParseSingle()__. Organization of parsing process is aided to perform identical processing text and binary messages.

## Binary commands parsing

Binary frames has unified header included in each frame:

***
| Field shortcut | Field length | Field name | Description |
|:--------------:|:------------:|:----------:|:-----------:|
| FC | 1 | Frame Code | defined in _bin_const.h_ file
| FL | 1 | Frame length | in bytes, including header fields so minimal value is 2
| *data | 0.. | | optional frame extra data field organized into structure with name same as FC and "_s" suffix, it is highly recommended to manually pack data structures
***

After call __BinParse()__ frame _FC_ is searched in __prot_cb_tab__ array. Each callback should have name same as FC but with _'_CB'_ suffix. In callbacks should check if source address is __ADDR_BROADCAST__ then target is local machine otherwise frame should be send to the given address. Printing to host (via USB, UART, SD-Card...) should be realized by **PRINT** module to ensure easy output format change.

## Summary

Text messages are converted to binary and during parsing binary frames frame content is printed via **PRINT** module if it is local frame or sent to remote device otherwise.