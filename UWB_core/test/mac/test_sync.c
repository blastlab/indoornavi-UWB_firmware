#include "unity.h"
#include "sync.h"

#include "mac.h"
#include "mock_transceiver.h"
#include "mock_deca_device_api.h"
FAKE_VALUE_FUNC(uint32_t, port_tick_hr);

settings_t settings = DEF_SETTINGS;
sync_instance_t sync;

void setUp(void)
{
}

void tearDown(void)
{
}

// static functions
uint64_t sync_glob_time(uint64_t dw_ts);
int toa_find_resp_ind(toa_core_t *toa);

void test_sync_rw_40b_values()
{
    uint8_t buf[10];
    memset(buf, 0, 10);

    sync_write_40b_value(buf, 0x1122334455);
    TEST_ASSERT_EQUAL_HEX64(0x1122334455, *(uint64_t *)buf);

    uint64_t result = sync_read_40b_value(buf);
    TEST_ASSERT_EQUAL_HEX64(0x1122334455, result);
}

void test_sync_sync_glob_time()
{
    uint64_t result;

    sync.local_obj.update_ts = 100;
    sync.local_obj.time_coeffP[0] = 0.01f;
    sync.local_obj.time_offset[0] = 400;
    result = sync_glob_time(1100);
    TEST_ASSERT_INT64_WITHIN(1, 1100 + 10 + 400, result);

    sync.local_obj.update_ts = 100;
    sync.local_obj.time_coeffP[0] = 0.0f;
    sync.local_obj.time_offset[0] = MASK_40BIT;
    result = sync_glob_time(10);
    TEST_ASSERT_INT64_WITHIN(1, 9, result);
}

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
    TEST_ASSERT_EQUAL(MAC_SYNC_MAX_AN_CNT, toa_find_resp_ind(&toa));
}