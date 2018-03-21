#include "unity.h"
#include "sync.h"

#include "mac.h"
#include "toa.h"
#include "mock_transceiver.h"
#include "mock_deca_device_api.h"
FAKE_VALUE_FUNC(uint32_t, port_tick_hr);
FAKE_VALUE_FUNC(int, _toa_get_range_bias, uint8, int, uint8, int);

int log_text(char type, const char *frm, ...)
{
    va_list arg;
    va_start(arg, frm);
    vprintf(frm, arg);
    va_end(arg);
}

settings_t settings = DEF_SETTINGS;
sync_instance_t sync;

void setUp(void)
{
}

void tearDown(void)
{
}

// static functions
int64_t sync_glob_time(int64_t dw_ts);

void test_sync_rw_40b_values()
{
    uint8_t buf[10];
    memset(buf, 0, 10);

    toa_write_40b_value(buf, 0x1122334455);
    TEST_ASSERT_EQUAL_HEX64(0x1122334455, *(int64_t *)buf);

    int64_t result = toa_read_40b_value(buf);
    TEST_ASSERT_EQUAL_HEX64(0x1122334455, result);
}

void test_sync_sync_glob_time()
{
    int64_t result;

    sync.local_obj.update_ts = 100;
    sync.local_obj.time_coeffP[0] = 0.01f;
    sync.local_obj.time_offset = 400;
    result = sync_glob_time(1100);
    TEST_ASSERT_INT64_WITHIN(1, 1100 + 10 + 400, result);

    sync.local_obj.update_ts = 100;
    sync.local_obj.time_coeffP[0] = 0.0f;
    sync.local_obj.time_offset = MASK_40BIT;
    result = sync_glob_time(10);
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