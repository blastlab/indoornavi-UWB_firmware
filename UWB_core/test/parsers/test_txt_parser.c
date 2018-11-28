#include "txt_parser.h"
#include "unity.h"

#include "logs.h"

#define TEST_ASSERT_M(EXPR) TEST_ASSERT_MESSAGE(EXPR, #EXPR)

TEST_FILE("logs_common.c")
TEST_FILE("iassert.c")

typedef const txt_buf_t * cp_txt_buf_t;
typedef const prot_packet_info_t * cp_prot_packet_info_t;

//FAKE_VOID_FUNC(txt_stat_cbb, cp_txt_buf_t, cp_prot_packet_info_t);

int stat_call = 0;
int stat_src = 0;
void txt_stat_cb(cp_txt_buf_t a, cp_prot_packet_info_t b) {
  ++stat_call;
  stat_src = b->original_src;
}

const txt_cb_t txt_cb_tab[] = {{"stat", txt_stat_cb}};

const int txt_cb_len = sizeof(txt_cb_tab) / sizeof(*txt_cb_tab);

void setUp(void) {}

void tearDown(void) {}

void test_txt_parser_check_flag(void) {
  char buffer[16] = "blah -p -mp";
  txt_buf_t buf = {
    .cmd = buffer,
    .start = buffer,
    .end = buffer + sizeof(buffer),
  };
  TEST_ASSERT_M(TXT_CheckFlag(&buf, "-m") == false);
  TEST_ASSERT_M(TXT_CheckFlag(&buf, "-p") == true);
  TEST_ASSERT_M(TXT_CheckFlag(&buf, "-mp") == true);
  TEST_ASSERT_M(TXT_CheckFlag(&buf, "-pm") == false);
}

void test_txt_parser_check_flag_on_buffer_edge(void) {
  char buffer[] = "p\0 blah -m";
  txt_buf_t buf = {
    .cmd = buffer+3,
    .start = buffer,
    .end = buffer + sizeof(buffer)-1, // obetnij zero na koncu bufora
  };

  TEST_ASSERT_M(TXT_CheckFlag(&buf, "-m") == false);
  TEST_ASSERT_M(TXT_CheckFlag(&buf, "-p") == false);
  TEST_ASSERT_M(TXT_CheckFlag(&buf, "-mp") == true);
  TEST_ASSERT_M(TXT_CheckFlag(&buf, "-pm") == !true);
}

void test_txt_parser_check_flag_on_buffer_edge2(void) {
  char buffer[] = "-mp\0 ble2 ";
  txt_buf_t buf = {
    .cmd = buffer+5,
    .start = buffer,
    .end = buffer + sizeof(buffer)-1, // obetnij zero na koncu bufora
  };

  TEST_ASSERT_M(TXT_CheckFlag(&buf, "-m") == false);
  TEST_ASSERT_M(TXT_CheckFlag(&buf, "-p") == false);
  TEST_ASSERT_M(TXT_CheckFlag(&buf, "-mp") == true);
  TEST_ASSERT_M(TXT_CheckFlag(&buf, "-pm") == false);
}

void test_txt_parser_GetParam(void) {
  char buffer[] = "-m 10\0 ble2 ";
  txt_buf_t buf = {
    .cmd = buffer+6,
    .start = buffer,
    .end = buffer + sizeof(buffer)-1, // obetnij zero na koncu bufora
  };

  TEST_ASSERT_M(TXT_GetParam(&buf, "-m", 10) == 10);
  TEST_ASSERT_M(TXT_GetParam(&buf, "-m", 16) == 0x10);
  TEST_ASSERT_M(TXT_GetParam(&buf, "-mp", 16) == -1);
}

void test_txt_parser_GetParam_hex_small_alpha(void) {
  char buffer[] = "m: 1b\0 ble2 ";
  txt_buf_t buf = {
    .cmd = buffer+7,
    .start = buffer,
    .end = buffer + sizeof(buffer)-1, // obetnij zero na koncu bufora
  };

  TEST_ASSERT_M(TXT_GetParam(&buf, "m:", 16) == 0x1b);
}

void test_txt_parser_GetParam_hex_big_alpha(void) {
  char buffer[] = "m:AC3\0 ble2 ";
  txt_buf_t buf = {
    .cmd = buffer+6,
    .start = buffer,
    .end = buffer + sizeof(buffer)-1, // obetnij zero na koncu bufora
  };

  TEST_ASSERT_M(TXT_GetParam(&buf, "m:", 16) == 0xac3);
}

void test_txt_parser_GetParamNum(void) {
  char buffer[] = "AC3\0 ble2 10 ";
  txt_buf_t buf = {
    .cmd = buffer+6,
    .start = buffer,
    .end = buffer + sizeof(buffer)-1, // obetnij zero na koncu bufora
  };

  TEST_ASSERT_M(TXT_GetParamNum(&buf, 1, 10) == 10);
  TEST_ASSERT_M(TXT_GetParamNum(&buf, 1, 16) == 0x10);
  TEST_ASSERT_M(TXT_GetParamNum(&buf, 2, 16) == 0xac3);
}

void test_txt_parser_StartsWith(void) {
  char buffer[] = "yuiop \0qwert";
  txt_buf_t buf = {
    .cmd = buffer+7,
    .start = buffer,
    .end = buffer + sizeof(buffer)-1, // obetnij zero na koncu bufora
  };

  TEST_ASSERT_M(TXT_StartsWith(&buf, "qwe") == true);
  TEST_ASSERT_M(TXT_StartsWith(&buf, "qwertyuio") == true);
}

void test_txt_parser_Input_to_Parse() {
  const char* cmd = "stat did:12\r\n";

  TXT_Input(cmd, strlen(cmd)-1);
  TXT_Control();
  TEST_ASSERT(stat_call == 0);

  
  TXT_Input("\n", 1);
  TXT_Control();
  TEST_ASSERT(stat_call == 1);
  TEST_ASSERT(stat_src == 0x12);
}