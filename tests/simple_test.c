/* tests/simple_test.c */
#include "unity.h"
#include <stdio.h>

/**
 * @file simple_test.c
 * @brief Simple test to verify Unity framework works
 */

void setUp(void) {
    /* Empty setup */
}

void tearDown(void) {
    /* Empty teardown */
}

void test_simple_assertion(void) {
    TEST_ASSERT_EQUAL_INT(42, 42);
    TEST_ASSERT_TRUE(1 == 1);
    TEST_ASSERT_FALSE(0 == 1);
}

void test_string_comparison(void) {
    TEST_ASSERT_EQUAL_STRING("hello", "hello");
    TEST_ASSERT_NOT_NULL("test");
    TEST_ASSERT_NULL(NULL);
}

void test_numeric_comparisons(void) {
    TEST_ASSERT_EQUAL_HEX8(0xAB, 0xAB);
    TEST_ASSERT_EQUAL_HEX16(0x1234, 0x1234);
    TEST_ASSERT_EQUAL_HEX32(0x12345678, 0x12345678);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_simple_assertion);
    RUN_TEST(test_string_comparison);
    RUN_TEST(test_numeric_comparisons);
    
    return UNITY_END();
}
