#include "unity.h"
#include "mac.h"

#include "logs.h"

#include "mock_transceiver.h"
#include "mock_port.h"
settings_t settings = DEF_SETTINGS;
#define TEST_ASSERT_M(expr) TEST_ASSERT_MESSAGE(expr, #expr)

void setUp(void)
{
}

void tearDown(void)
{
}

void test_mac_NeedToImplement(void)
{
    TEST_IGNORE_MESSAGE("Need to Implement mac");
}

void test_mac_read_write_zero_len()
{
    const char data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    unsigned char rbuf[100];
    mac_buf_t *buf = mac_buffer();
    // write zero len
    uint8_t idata = *buf->dPtr;
    uint8_t *idPtr = buf->dPtr;
    mac_write(buf, data + 1, 0);
    mac_write(buf, data + 1, 0);
    TEST_ASSERT_M(buf->dPtr == idPtr);
    TEST_ASSERT_M(*buf->dPtr == idata);
    memset(rbuf, 0xAA, 100);
    TEST_ASSERT_M(*rbuf == 0xAA);
    mac_read(buf, rbuf, 0);
    TEST_ASSERT_M(buf->dPtr == idPtr);
    TEST_ASSERT_M(rbuf[0] == 0xAA);
}

void test_mac_read_write_positive_len()
{
    const uint8_t data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint8_t rbuf[100];
    mac_buf_t *buf;
    for (int i = 0; i < 10; ++i)
    {
        buf = mac_buffer();
        TEST_ASSERT_M(buf != 0);
        mac_write(buf, data, i);
        buf->dPtr = buf->buf; // reset read dPtr
        mac_read(buf, rbuf, i);
        TEST_ASSERT_M(memcmp(rbuf, data, i) == 0);
        mac_free(buf);
    }
}