/**
 * @brief base64 coder and decoder in C
 *
 * @file base64.h
 * @date 2018-06-28
 * @author: Karol Trzcinski
 */

#ifndef UWB_BASE64_H_
#define UWB_BASE64_H_

/**
 * @brief from bin to base64
 *
 * @note function add terminating 0 at the end of @p dst
 * @note @p dst can NOT point same place as @p dst
 *
 * @param[out] dst destination buffer for base64 string
 * @param[in] src source buffer with binary data
 * @param[in] srcSize size of source data to encode
 * @return int dst buffer size
 */
int BASE64_Encode(unsigned char* dst, const unsigned char* src, unsigned short srcSize);

/**
 * @brief from base64 to bin
 *
 * decode src buff to \0 char or to first occurrence of '='
 *
 * @note @p dst can point same place as @p dst
 *
 * @param[out] dst destination buffer for binary data
 * @param[in] src source buffer for
 * @param[in] dstBufCapacity maximal destination buffer capacity
 * @return int size of bytes in dst buffer without terminating zero
 */
int BASE64_Decode(unsigned char* dst, const unsigned char* src, unsigned short dstBufCapacity);

/**
 * @brief calculate target base64 text size
 *
 * @param binSize binary data size in bytes
 * @return int base64 text size in bytes (without null terminator)
 */
int BASE64_TextSize(unsigned int binSize);

#endif /* UWB_BASE64_H_ */
