#include "unity.h"

// mocks
#include "mock_deca_device_api.h"
#include "mock_port.h"
#include "transceiver.h"
settings_t settings = DEF_SETTINGS;

void setUp(void) {}

void tearDown(void) {}

void test_transceiver_NeedToImplement(void) {
  TEST_IGNORE_MESSAGE("Need to Implement transceiver");
}

void test_TRANSCEIVER_Init() {
  RESET_FAKE(PORT_ResetTransceiver);
  RESET_FAKE(dwt_initialise);
  RESET_FAKE(dwt_configure);
  RESET_FAKE(PORT_SpiSpeedSlow);

  uint32 readdevid_seq[] = {DWT_DEVICE_ID};
  int readdevid_len = sizeof(readdevid_seq) / sizeof(*readdevid_seq);
  int initialise_seq[] = {DWT_DEVICE_ID};
  int initialise_len = sizeof(readdevid_seq) / sizeof(*readdevid_seq);
  SET_RETURN_SEQ(dwt_readdevid, readdevid_seq, readdevid_len);
  SET_RETURN_SEQ(dwt_initialise, initialise_seq, initialise_len);

  TRANSCEIVER_Init(12, 34);

  TEST_ASSERT(PORT_ResetTransceiver_fake.call_count == 1);
  TEST_ASSERT(dwt_initialise_fake.call_count == 1);
  TEST_ASSERT(dwt_configure_fake.call_count == 1);
  TEST_ASSERT(PORT_SpiSpeedSlow_fake.call_count == 2);
}
