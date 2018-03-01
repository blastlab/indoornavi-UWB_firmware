#include "unity.h"
#include "mac.h"

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

void test_mac_read_write_consistent()
{
    const char data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    char rbuf[100];
    //mac_buf_t *buf = mac_buffer();
}