#include "sync.h"
#include "unity.h"

#include <math.h>

#include "logs.h"
#include "mac.h"
#include "mock_port.h"
#include "mock_transceiver.h"
#include "mock_toa_routine.h"
#include "toa.h"

TEST_FILE("iassert.c")
TEST_FILE("logs_common.c")
//TEST_FILE("mac.c")

settings_otp_t _settings_otp;
settings_otp_t const *settings_otp = &_settings_otp;

FAKE_VALUE_FUNC(int, _TOA_GetRangeBias, uint8, int, uint8, int);
FAKE_VOID_FUNC(CARRY_ParseMessage, mac_buf_t *);
FAKE_VOID_FUNC(dwt_forcetrxoff);
FAKE_VOID_FUNC(dwt_rxreset);
FAKE_VOID_FUNC(dwt_setrxtimeout, uint16_t);
FAKE_VOID_FUNC(dwt_setrxaftertxdelay, unsigned long);
FAKE_VOID_FUNC(dwt_setdelayedtrxtime, unsigned long);
FAKE_VALUE_FUNC(int, dwt_rxenable, int);
FAKE_VOID_FUNC(listener_isr, const dwt_cb_data_t *)
FAKE_VOID_FUNC(FU_AcceptFirmware);



settings_t settings = DEF_SETTINGS;
sync_instance_t sync;

void setUp(void) {}

void tearDown(void) {}

// static functions
int64_t SYNC_GlobTime(int64_t dw_ts);

void test_sync_rw_40b_values() {
  uint8_t buf[10];
  memset(buf, 0, 10);

  TOA_Write40bValue(buf, 0x1122334455);
  TEST_ASSERT_EQUAL_HEX64(0x1122334455, *(int64_t *)buf);

  int64_t result = TOA_Read40bValue(buf);
  TEST_ASSERT_EQUAL_HEX64(0x1122334455, result);
}

void test_sync_SYNC_GlobTime() {
  int64_t result;

  sync.local_obj.update_ts = 100;
  sync.local_obj.time_coeffP[0] = 0.01f;
  sync.local_obj.time_offset = 400;
  result = SYNC_GlobTime(1100);
  TEST_ASSERT_INT64_WITHIN(1, 1100 + 10 + 400, result);

  sync.local_obj.update_ts = 100;
  sync.local_obj.time_coeffP[0] = 0.0f;
  sync.local_obj.time_offset = MASK_40BIT;
  result = SYNC_GlobTime(10);
  TEST_ASSERT_INT64_WITHIN(1, 9, result);
}

/*
void test_sync_find_resp_ind()
{
    toa_core_t toa;

    toa.anc_in_poll_cnt = 2;
    toa.resp_ind = 7;
    toa.addr_tab[0] = 1;
    toa.addr_tab[1] = 2;

    settings.mac.addr = 1;
    TEST_ASSERT_EQUAL(0, toa_find_resp_ind(&toa));

    settings.mac.addr = 2;
    TEST_ASSERT_EQUAL(1, toa_find_resp_ind(&toa));

    settings.mac.addr = 3;
    TEST_ASSERT_EQUAL(TOA_MAX_DEV_IN_POLL, toa_find_resp_ind(&toa));
}*/