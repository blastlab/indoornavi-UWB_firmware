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
int BASE64_Encode(unsigned char *dst, const unsigned char *src,
                  unsigned short srcSize);

// return size of bytes in dst buffer without terminating zero
// function decode src buff to \0 char or to first occurence of '='
int BASE64_Decode(unsigned char *dst, const unsigned char *src,
                  unsigned short dstBufCapacity);

// return base64 text size (without null terminator) based on given binary size
int BASE64_TextSize(unsigned int binSize);

void base64_test();

#endif /* UWB_BASE64_H_ */
