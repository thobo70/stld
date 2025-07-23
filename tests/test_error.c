/* tests/test_error.c */
#include "unity.h"
#include "error.h"
#include <stdio.h>
#include <string.h>

/**
 * @file test_error.c
 * @brief Unit tests for error handling system
 * @details Tests error reporting, message formatting, and callback handling
 */

/* Function prototypes for C90 compliance */
void test_error_code_to_string(void);
void test_error_is_fatal(void);
void test_error_is_success_failure(void);
void test_error_is_warning(void);
void test_error_format_message(void);
void test_error_format_message_small_buffer(void);
void test_error_callback_setting(void);
void test_error_report_with_callback(void);
int test_error_main(void);

/* Test callback storage */
static error_context_t last_error_context;
static bool callback_called = false;

void setUp(void) {
    /* Reset test state before each test */
    memset(&last_error_context, 0, sizeof(last_error_context));
    callback_called = false;
    error_set_callback(NULL);
}

void tearDown(void) {
    /* Clean up after each test */
    error_set_callback(NULL);
}

/* Test callback function */
static void test_error_callback(const error_context_t* context) {
    last_error_context = *context;
    callback_called = true;
}

void test_error_code_to_string(void) {
    TEST_ASSERT_EQUAL_STRING("Success", error_get_string(ERROR_SUCCESS));
    TEST_ASSERT_EQUAL_STRING("Out of memory", error_get_string(ERROR_OUT_OF_MEMORY));
    TEST_ASSERT_EQUAL_STRING("Invalid argument", error_get_string(ERROR_INVALID_ARGUMENT));
    TEST_ASSERT_EQUAL_STRING("Invalid magic number", error_get_string(ERROR_INVALID_MAGIC));
    TEST_ASSERT_EQUAL_STRING("Unsupported version", error_get_string(ERROR_UNSUPPORTED_VERSION));
    TEST_ASSERT_EQUAL_STRING("File not found", error_get_string(ERROR_FILE_NOT_FOUND));
    TEST_ASSERT_EQUAL_STRING("Symbol not found", error_get_string(ERROR_SYMBOL_NOT_FOUND));
    TEST_ASSERT_EQUAL_STRING("Invalid relocation", error_get_string(ERROR_INVALID_RELOCATION));
    TEST_ASSERT_EQUAL_STRING("File I/O error", error_get_string(ERROR_FILE_IO));
    TEST_ASSERT_EQUAL_STRING("Unknown error", error_get_string((error_code_t)999)); /* Invalid code */
}

void test_error_is_fatal(void) {
    /* Test which errors are considered fatal */
    TEST_ASSERT_FALSE(error_is_fatal(ERROR_SUCCESS));
    TEST_ASSERT_TRUE(error_is_fatal(ERROR_OUT_OF_MEMORY));
    TEST_ASSERT_TRUE(error_is_fatal(ERROR_INTERNAL));
    TEST_ASSERT_FALSE(error_is_fatal(ERROR_SYMBOL_NOT_FOUND));
}

void test_error_is_success_failure(void) {
    /* Test success/failure checking */
    TEST_ASSERT_TRUE(error_is_success(ERROR_SUCCESS));
    TEST_ASSERT_FALSE(error_is_success(ERROR_OUT_OF_MEMORY));
    
    TEST_ASSERT_FALSE(error_is_failure(ERROR_SUCCESS));
    TEST_ASSERT_TRUE(error_is_failure(ERROR_OUT_OF_MEMORY));
    TEST_ASSERT_TRUE(error_is_failure(ERROR_INVALID_ARGUMENT));
}

void test_error_is_warning(void) {
    /* Test warning detection */
    TEST_ASSERT_FALSE(error_is_warning(ERROR_SUCCESS));
    TEST_ASSERT_TRUE(error_is_warning(ERROR_INVALID_ARGUMENT));
    TEST_ASSERT_TRUE(error_is_warning(ERROR_SYMBOL_NOT_FOUND));
    TEST_ASSERT_FALSE(error_is_warning(ERROR_INTERNAL));
}

void test_error_format_message(void) {
    char buffer[256];
    int result;
    
    result = error_format_message(buffer, sizeof(buffer), "Test %s %d", "message", 42);
    
    TEST_ASSERT_TRUE(result > 0);
    TEST_ASSERT_EQUAL_STRING("Test message 42", buffer);
}

void test_error_format_message_small_buffer(void) {
    char small_buffer[5];
    int result;
    
    result = error_format_message(small_buffer, sizeof(small_buffer), 
                                "Very long message that will be truncated");
    
    /* Should truncate and null-terminate */
    TEST_ASSERT_TRUE(result >= 0);
    TEST_ASSERT_EQUAL_UINT(0, (uint32_t)small_buffer[sizeof(small_buffer) - 1]);
}

void test_error_callback_setting(void) {
    error_callback_t callback;
    
    /* Test setting and getting callback */
    TEST_ASSERT_TRUE(error_get_callback() == NULL);
    
    error_set_callback(test_error_callback);
    callback = error_get_callback();
    TEST_ASSERT_TRUE(callback == test_error_callback);
    
    error_set_callback(NULL);
    TEST_ASSERT_TRUE(error_get_callback() == NULL);
}

void test_error_report_with_callback(void) {
    /* Set callback and report an error */
    error_set_callback(test_error_callback);
    
    error_report(ERROR_INVALID_ARGUMENT, ERROR_SEVERITY_ERROR,
                "test_file.c", 100, "test_function", "Test error message");
    
    /* Verify callback was called with correct parameters */
    TEST_ASSERT_TRUE(callback_called);
    TEST_ASSERT_EQUAL_INT(ERROR_INVALID_ARGUMENT, last_error_context.code);
    TEST_ASSERT_EQUAL_INT(ERROR_SEVERITY_ERROR, last_error_context.severity);
    TEST_ASSERT_EQUAL_STRING("Test error message", last_error_context.message);
    TEST_ASSERT_EQUAL_STRING("test_file.c", last_error_context.file);
    TEST_ASSERT_EQUAL_INT(100, last_error_context.line);
    TEST_ASSERT_EQUAL_STRING("test_function", last_error_context.function);
}

int test_error_main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_error_code_to_string);
    RUN_TEST(test_error_is_fatal);
    RUN_TEST(test_error_is_success_failure);
    RUN_TEST(test_error_is_warning);
    RUN_TEST(test_error_format_message);
    RUN_TEST(test_error_format_message_small_buffer);
    RUN_TEST(test_error_callback_setting);
    RUN_TEST(test_error_report_with_callback);
    
    return UNITY_END();
}

int main(void) {
    return test_error_main();
}
