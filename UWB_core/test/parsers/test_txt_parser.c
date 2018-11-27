#include "txt_parser.h"
#include "unity.h"

#include "logs.h"

TEST_FILE("logs_common.c")
TEST_FILE("iassert.c")

typedef const txt_buf_t *cp_txt_buf_t;
typedef const prot_packet_info_t *cp_prot_packet_info_t;
// FAKE_VOID_FUNC(txt_stat_cb, cp_txt_buf_t, cchar *, cp_prot_packet_info_t);

void txt_stat_cb(cp_txt_buf_t a, cp_prot_packet_info_t b) {}

const txt_cb_t txt_cb_tab[] = {{"stat", txt_stat_cb}};

const int txt_cb_len = sizeof(txt_cb_tab) / sizeof(*txt_cb_tab);

void setUp(void) {}

void tearDown(void) {}

void test_txt_parser_check_flag(void) {
  char buffer[16] = "blah -m -p -mp";
  txt_buf_t buf = {
    .cmd = buffer,
    .start = buffer,
    .end = buffer + sizeof(buffer),
  };
  /*TEST_ASSERT(TXT_CheckFlag(&buf, "-m") == true);
  TEST_ASSERT(TXT_CheckFlag(&buf, "-p") == true);
  TEST_ASSERT(TXT_CheckFlag(&buf, "-mp") == true);
  TEST_ASSERT(TXT_CheckFlag(&buf, "-pm") == !true);*/
}

void test_txt_parser_NeedToImplement(void) {
  TEST_IGNORE_MESSAGE("Need to Implement txt_parser");
}
