#include "txt_parser.h"
#include "unity.h"

#include "logs.h"

typedef const txt_buf_t *cp_txt_buf_t;
typedef const prot_packet_info_t *cp_prot_packet_info_t;
// FAKE_VOID_FUNC(txt_stat_cb, cp_txt_buf_t, cchar *, cp_prot_packet_info_t);

void txt_stat_cb(cp_txt_buf_t a, cp_prot_packet_info_t b) {}

const txt_cb_t txt_cb_tab[] = {{"stat", txt_stat_cb}};

const int txt_cb_len = sizeof(txt_cb_tab) / sizeof(*txt_cb_tab);

void setUp(void) {}

void tearDown(void) {}

void test_txt_parser_NeedToImplement(void) {
  TEST_IGNORE_MESSAGE("Need to Implement txt_parser");
}
