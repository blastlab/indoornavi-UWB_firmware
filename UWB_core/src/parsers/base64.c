/*
 * base64.c
 *
 *  Created on: 20 cze 2017
 *      Author: Karol Trzcinski
 */

#include "base64.h"

// char in ASCII to unsigned char in dec
static unsigned char BASE64_CtoI(unsigned char c) {
	if (c >= 'A' && c <= 'Z') {
		return c - 'A';
	} else if (c >= 'a' && c <= 'z') {
		return c - 'a' + 26;
	} else if (c >= '0' && c <= '9') {
		return c - '0' + 52;
	} else if (c == '+' || c == '-') {
		return 62;
	} else if (c == '/' || c == '_') {
		return 63;
	} else if (c == 0) {
		return 0;
	} else {
		return 0xff;
	}
}

// convert unsigned char in dec to base64 ASCII
static unsigned char BASE64_ItoC(unsigned char s) {
	if (s < 26)
		return s + 'A';
	else if (s < 52) {
		return s - 26 + 'a';
	} else if (s < 62) {
		return s - 52 + '0';
	} else if (s == 62) {
		return '+';
	} else if (s == 63) {
		return '/';
	} else
		return '=';
}

// return 1 if it is end of base64 char
static unsigned char BASE64_EndC(unsigned char c) {
	if (c == '=' || c == ' ' || c == 0) {
		return 1;
	} else {
		return 0;
	}
}

// return base64 text size (without null terminator) based on given binary size
int BASE64_TextSize(unsigned int binSize) {
	binSize = (binSize % 3) == 0 ? (binSize / 3) : (binSize / 3 + 1);
	return binSize * 4;
}

// from bin to base64
// return dst buffer size
// function add terminating 0 at the end of dst
int BASE64_Encode(unsigned char* dst, const unsigned char* src, unsigned short srcSize) {
#ifdef BASE64_TEST
	assert_pram(dst != src);
#endif
	unsigned char* dst_base = dst;
	int s = 0;
	while (s < srcSize) {
		unsigned char a = s + 0 < srcSize ? src[s + 0] : 0;
		unsigned char b = s + 1 < srcSize ? src[s + 1] : 0;
		unsigned char c = s + 2 < srcSize ? src[s + 2] : 0;
		unsigned int r = ((unsigned int)a << 16) + ((unsigned int)b << 8) + c;
		--s;
		for (signed char i = 18; i >= 0; i -= 6) {
			*dst = BASE64_ItoC((r >> i) & 0x3F);
			++dst;
			if (++s >= srcSize) {
				break;
			}
		}
	}
	// add padding
	s = dst - dst_base;
	while (s % 4) {
		*dst = '=';
		++dst;
		++s;
	}
	*dst = 0;
	return s;
}

// from base64 to bin
// return size of bytes in dst buffer without terminating zero
// function decode src buff to \0 char or to first occurence of '='
int BASE64_Decode(unsigned char* dst, const unsigned char* src, unsigned short dstBufCapacity) {
	unsigned char* base_dst = dst;
	unsigned char tmp[4];
	register int i;
	for (register int d = 0; BASE64_EndC(src[0]) == 0; d += 3, src += 4) {
		// przekonweruj ascii do bin, nie przechodzac przez znak konca napisu
		for (i = 0; i < 4; ++i) {
			tmp[i] = BASE64_CtoI(src[i]);
		}

		// decode
		*(dst++) = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
		if (BASE64_EndC(src[2])) {
			break;
		} else {
			*(dst++) = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
			if (BASE64_EndC(src[3])) {
				break;  // end of characters
			} else {
				*(dst++) = ((tmp[2] & 0x3) << 6) + tmp[3];
			}
		}
	}

	return dst - base_dst;  // size of bytes in dst buffer
}
