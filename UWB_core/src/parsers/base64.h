/*
 * base64.h
 *
 *  Created on: 12.07.2017
 *      Author: KarolTrzcinski
 */

#ifndef UWB_BASE64_H_
#define UWB_BASE64_H_

// return dst buffer size
// function add terminating 0 at the end of dst
int base64_encode(unsigned char *dst, const unsigned char *src,
                  unsigned short srcSize);

// return size of bytes in dst buffer without terminating zero
// function decode src buff to \0 char or to first occurence of '='
int base64_decode(unsigned char *dst, const unsigned char *src,
                  unsigned short dstBufCapacity);

// return base64 text size (without null terminator) based on given binary size
int base64_textSize(unsigned int binSize);

void base64_test();

#endif /* UWB_BASE64_H_ */
