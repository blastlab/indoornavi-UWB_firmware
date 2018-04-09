/*
 * bootloader.s
 *
 *  Created on: 29.09.2017
 *      Author: KarolTrzcinski
 */


.section .bootloader,"ax",%progbits
Bootloader:
	.incbin "../L4Bootloader.bin"
