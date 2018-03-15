#include "unity.h"
#include "transceiver.h"

// mocks
#include "mock_deca_device_api.h"
FAKE_VOID_FUNC(reset_DW1000);
FAKE_VOID_FUNC(spi_speed_slow, bool);
FAKE_VALUE_FUNC(uint32_t, port_tick_hr);
settings_t settings = DEF_SETTINGS;

void setUp(void)
{
}

void tearDown(void)
{
}

void test_transceiver_NeedToImplement(void)
{
    TEST_IGNORE_MESSAGE("Need to Implement transceiver");
}

void test_transceiver_init()
{
    RESET_FAKE(reset_DW1000);
    RESET_FAKE(dwt_initialise);
    RESET_FAKE(dwt_configure);
    RESET_FAKE(spi_speed_slow);

    uint32 readdevid_seq[] = {DWT_DEVICE_ID};
    int readdevid_len = sizeof(readdevid_seq) / sizeof(*readdevid_seq);
    int initialise_seq[] = {DWT_DEVICE_ID};
    int initialise_len = sizeof(readdevid_seq) / sizeof(*readdevid_seq);
    SET_RETURN_SEQ(dwt_readdevid, readdevid_seq, readdevid_len);
    SET_RETURN_SEQ(dwt_initialise, initialise_seq, initialise_len);

    transceiver_init(12, 34);

    TEST_ASSERT(reset_DW1000_fake.call_count == 1);
    TEST_ASSERT(dwt_initialise_fake.call_count == 1);
    TEST_ASSERT(dwt_configure_fake.call_count == 1);
    TEST_ASSERT(spi_speed_slow_fake.call_count == 2);
}
