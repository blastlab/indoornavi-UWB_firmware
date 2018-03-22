#include "unity.h"
#include "parsers/text_parser.h"

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

void test_text_NeedToImplement(void)
{
    TEST_IGNORE_MESSAGE("Need to Implement mac");
}