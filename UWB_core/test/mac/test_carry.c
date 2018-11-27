#include "mac/carry.h"
#include "unity.h"


#include "mock_port.h"
#include "mock_transceiver.h"
#include "mock_mac.h"

#include "logs.h"
//#include "mac.h"

TEST_FILE("iassert.c")
TEST_FILE("logs_common.c")

settings_t settings = DEF_SETTINGS;

FAKE_VALUE_FUNC(int, LOG_Bin, const void*, int);
FAKE_VALUE_FUNC(const uint8_t*, BIN_Parse, const uint8_t *, const prot_packet_info_t*, int);

// FAKE_VALUE_FUNC(unsigned int, HAL_GetTick);
// FAKE_VALUE_FUNC(mac_buff_time_t, mac_port_buff_time);

void setUp(void) {}

void tearDown(void) {}

void test_carry_NeedToImplement(void) {
  TEST_IGNORE_MESSAGE("Need to Implement carry");
}
