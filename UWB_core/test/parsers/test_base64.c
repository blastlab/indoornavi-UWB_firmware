#include "base64.h"
#include "unity.h"

#include <string.h>

#define DW_ASSERT TEST_ASSERT

void setUp(void) {}

void tearDown(void) {}

void test_BASE64_Decode_short() {
  unsigned char buf[128], buf1[128];
  int buf_size = sizeof(buf) / sizeof(*buf);
  int s;

  s = BASE64_Decode(buf, (unsigned char *)"", buf_size);
  DW_ASSERT(memcmp(buf, "", s) == 0);
  DW_ASSERT(s == 0);

  s = BASE64_Decode(buf, (unsigned char *)"=", buf_size);
  DW_ASSERT(memcmp(buf, "", s) == 0);
  DW_ASSERT(s == 0);

  s = BASE64_Decode(buf, (unsigned char *)"==", buf_size);
  DW_ASSERT(memcmp(buf, "", s) == 0);
  DW_ASSERT(s == 0);

  s = BASE64_Decode(buf, (unsigned char *)"ZHppYWxh", buf_size);
  DW_ASSERT(memcmp(buf, "dziala", s) == 0);
  DW_ASSERT(s == 6);

  s = BASE64_Decode(buf, (unsigned char *)"S2Fyb2w=", buf_size);
  DW_ASSERT(memcmp(buf, "Karol", s) == 0);
  DW_ASSERT(s == 5);

  s = BASE64_Decode(buf, (unsigned char *)"SmVzdCBPaw==", buf_size);
  DW_ASSERT(memcmp(buf, "Jest Ok", s) == 0);
  DW_ASSERT(s == 7);
}

void test_BASE64_Decode_InPlace() {
  unsigned char buf[128];
  int buf_size = sizeof(buf) / sizeof(*buf);
  int s;

  strcpy((char *)buf, "");
  s = BASE64_Decode(buf, buf, buf_size);
  DW_ASSERT(memcmp(buf, "", s) == 0);
  DW_ASSERT(s == 0);

  strcpy((char *)buf, "=");
  s = BASE64_Decode(buf, buf, buf_size);
  DW_ASSERT(memcmp(buf, "", s) == 0);
  DW_ASSERT(s == 0);

  strcpy((char *)buf, "==");
  s = BASE64_Decode(buf, buf, buf_size);
  DW_ASSERT(memcmp(buf, "", s) == 0);
  DW_ASSERT(s == 0);

  strcpy((char *)buf, "ZHppYWxh");
  s = BASE64_Decode(buf, buf, buf_size);
  DW_ASSERT(memcmp(buf, "dziala", s) == 0);
  DW_ASSERT(s == 6);

  strcpy((char *)buf, "S2Fyb2w=");
  s = BASE64_Decode(buf, buf, buf_size);
  DW_ASSERT(memcmp(buf, "Karol", s) == 0);
  DW_ASSERT(s == 5);

  strcpy((char *)buf, "SmVzdCBPaw==");
  s = BASE64_Decode(buf, buf, buf_size);
  DW_ASSERT(memcmp(buf, "Jest Ok", s) == 0);
  DW_ASSERT(s == 7);
}

void test_BASE64_Decode_long() {
  unsigned char buf[128], buf1[128];
  int buf_size = sizeof(buf) / sizeof(*buf);
  int s;

  strcpy((char *)buf,
         "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
  s = BASE64_Decode(buf, buf, buf_size);
  const char res0[] = {
      0x00, 0x10, 0x83, 0x10, 0x51, 0x87, 0x20, 0x92, 0x8B, 0x30, 0xD3, 0x8F,
      0x41, 0x14, 0x93, 0x51, 0x55, 0x97, 0x61, 0x96, 0x9B, 0x71, 0xD7, 0x9F,
      0x82, 0x18, 0xA3, 0x92, 0x59, 0xA7, 0xA2, 0x9A, 0xAB, 0xB2, 0xDB, 0xAF,
      0xC3, 0x1C, 0xB3, 0xD3, 0x5D, 0xB7, 0xE3, 0x9E, 0xBB, 0xF3, 0xDF, 0xBF};
  DW_ASSERT(memcmp(buf, res0, s) == 0);

  strcpy((char *)buf, "Fk8A//"
                      "8AF0kAEzAAAADAACA5wQAIcbsACHW7AAh5uwAIfbsACIG7AAgAAAAAAA"
                      "AAAAAAAAAAAAAAhbsACIm7AAgAAAAAjbsACJG7AAhlAA==");
  DW_ASSERT(strlen((char *)buf) < buf_size);
  s = BASE64_Decode(buf, buf, buf_size);
  const uint8_t res1[] = {
      0x16, 0x4F, 0x00, 0xFF, 0xFF, 0x00, 0x17, 0x49, 0x00, 0x13, 0x30, 0x00,
      0x00, 0x00, 0xC0, 0x00, 0x20, 0x39, 0xC1, 0x00, 0x08, 0x71, 0xBB, 0x00,
      0x08, 0x75, 0xBB, 0x00, 0x08, 0x79, 0xBB, 0x00, 0x08, 0x7D, 0xBB, 0x00,
      0x08, 0x81, 0xBB, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x85, 0xBB, 0x00,
      0x08, 0x89, 0xBB, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x8D, 0xBB, 0x00,
      0x08, 0x91, 0xBB, 0x00, 0x08, 0x65, 0x00};
  DW_ASSERT(memcmp(buf, res1, s) == 0);
}

void test_BASE64_Encode_short() {
  unsigned char buf[128], buf1[128];
  int buf_size = sizeof(buf) / sizeof(*buf);
  int s;

  s = BASE64_Encode(buf, (uint8_t *)":->", 3);
  DW_ASSERT(memcmp(buf, "Oi0+", 4) == 0);
  DW_ASSERT(memcmp(buf, "Oi0+", 5) == 0); // sprawdz zero na koncu
  DW_ASSERT(s == 4);

  s = BASE64_Encode(buf, (uint8_t *)"Karol", 5);
  DW_ASSERT(memcmp(buf, "S2Fyb2w=", 8) == 0);
  DW_ASSERT(memcmp(buf, "S2Fyb2w=", 9) == 0); // sprawdz zero na koncu
  DW_ASSERT(s == 8);

  s = BASE64_Encode(buf, (uint8_t *)"Jest Ok", 7);
  DW_ASSERT(memcmp(buf, "SmVzdCBPaw==", s) == 0);
  DW_ASSERT(s == 12);
}

void test_BASE64_Decode_encode_cycle() {
  unsigned char buf[128], buf1[128];
  int buf_size = sizeof(buf) / sizeof(*buf);
  int s;

  const unsigned char str1[] = "FhcA//8AFxEAEjAYAAAYWeVw3AAAiUQ=";
  s = BASE64_Decode(buf, str1, buf_size);
  s = BASE64_Encode(buf1, buf, s);
  DW_ASSERT(memcmp(buf1, str1, s) == 0);

  const unsigned char str2[] = "Fk8A//"
                               "8AF0kAEzAAAADAACC9wQAI9bsACPm7AAj9uwAIAbwACAW8A"
                               "AgAAAAAAAAAAAAAAAAAAAAACbwACA28AAgAAAAAEbwACBW8"
                               "AAhKuQ==";
  s = BASE64_Decode(buf, str2, buf_size);
  s = BASE64_Encode(buf1, buf, s);
  DW_ASSERT(memcmp(buf1, str2, s) == 0);
}

void test_BASE64_TextSize() {
  DW_ASSERT(BASE64_TextSize(0) == 0);
  DW_ASSERT(BASE64_TextSize(1) == 4);
  DW_ASSERT(BASE64_TextSize(2) == 4);
  DW_ASSERT(BASE64_TextSize(3) == 4);
  DW_ASSERT(BASE64_TextSize(4) == 8);
  DW_ASSERT(BASE64_TextSize(5) == 8);
  DW_ASSERT(BASE64_TextSize(6) == 8);
  DW_ASSERT(BASE64_TextSize(7) == 12);
  DW_ASSERT(BASE64_TextSize(8) == 12);
}
