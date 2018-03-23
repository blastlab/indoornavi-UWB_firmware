#include "prot/carry.h"
#include "unity.h"


#include "mock_port.h"
#include "mock_transceiver.h"

#include "logs.h"
#include "mac.h"


settings_t settings = DEF_SETTINGS;

// FAKE_VALUE_FUNC(unsigned int, HAL_GetTick);
// FAKE_VALUE_FUNC(mac_buff_time_t, mac_port_buff_time);

void setUp(void) {}

void tearDown(void) {}

void test_carry_NeedToImplement(void) {
  TEST_IGNORE_MESSAGE("Need to Implement carry");
}
