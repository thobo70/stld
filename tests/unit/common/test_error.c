/**
 * @file test_error.c
 * @brief Unit tests for error handling module
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * Comprehensive unit tests for error handling, error code management,
 * error message formatting, and error propagation mechanisms.
 */

#include "unity.h"
#include "error.h"
#include <string.h>
#include <errno.h>

/* Test fixture data */
static error_context_t* test_context;

/**
 * @brief Setup function called before each test
 */
void setUp(void) {
    test_context = error_context_create();
    TEST_ASSERT_NOT_NULL(test_context);
}

/**
 * @brief Teardown function called after each test
 */
void tearDown(void) {
    if (test_context != NULL) {
        error_context_destroy(test_context);
        test_context = NULL;
    }
}

/**
 * @brief Test error context creation and destruction
 */
void test_error_context_lifecycle(void) {
    error_context_t* ctx = error_context_create();
    TEST_ASSERT_NOT_NULL(ctx);
    
    /* Initial state should be no error */
    TEST_ASSERT_EQUAL_INT(ERROR_SUCCESS, error_context_get_code(ctx));
    TEST_ASSERT_NULL(error_context_get_message(ctx));
    TEST_ASSERT_FALSE(error_context_has_error(ctx));
    
    error_context_destroy(ctx);
    
    /* Test destruction with NULL */
    error_context_destroy(NULL); /* Should not crash */
}

/**
 * @brief Test basic error setting and retrieval
 */
void test_error_basic_operations(void) {
    /* Set an error */
    error_context_set(test_context, ERROR_INVALID_PARAMETER, "Test error message");
    
    TEST_ASSERT_TRUE(error_context_has_error(test_context));
    TEST_ASSERT_EQUAL_INT(ERROR_INVALID_PARAMETER, error_context_get_code(test_context));
    
    const char* message = error_context_get_message(test_context);
    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_EQUAL_STRING("Test error message", message);
    
    /* Clear the error */
    error_context_clear(test_context);
    
    TEST_ASSERT_FALSE(error_context_has_error(test_context));
    TEST_ASSERT_EQUAL_INT(ERROR_SUCCESS, error_context_get_code(test_context));
    TEST_ASSERT_NULL(error_context_get_message(test_context));
}

/**
 * @brief Test error code validation
 */
void test_error_code_validation(void) {
    /* Test valid error codes */
    TEST_ASSERT_TRUE(error_code_is_valid(ERROR_SUCCESS));
    TEST_ASSERT_TRUE(error_code_is_valid(ERROR_INVALID_PARAMETER));
    TEST_ASSERT_TRUE(error_code_is_valid(ERROR_OUT_OF_MEMORY));
    TEST_ASSERT_TRUE(error_code_is_valid(ERROR_FILE_NOT_FOUND));
    TEST_ASSERT_TRUE(error_code_is_valid(ERROR_INVALID_FORMAT));
    TEST_ASSERT_TRUE(error_code_is_valid(ERROR_UNSUPPORTED_VERSION));
    
    /* Test invalid error codes */
    TEST_ASSERT_FALSE(error_code_is_valid((error_code_t)-1));
    TEST_ASSERT_FALSE(error_code_is_valid((error_code_t)1000));
}

/**
 * @brief Test error message formatting
 */
void test_error_message_formatting(void) {
    /* Test formatted error messages */
    error_context_setf(test_context, ERROR_INVALID_PARAMETER, 
                       "Invalid parameter at index %d: %s", 5, "filename");
    
    TEST_ASSERT_TRUE(error_context_has_error(test_context));
    
    const char* message = error_context_get_message(test_context);
    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_EQUAL_STRING("Invalid parameter at index 5: filename", message);
    
    /* Test long message formatting */
    error_context_setf(test_context, ERROR_FILE_NOT_FOUND,
                       "File not found: %s (searched in %d directories)",
                       "/very/long/path/to/some/file/that/does/not/exist.smof", 10);
    
    message = error_context_get_message(test_context);
    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_TRUE(strlen(message) > 50);
}

/**
 * @brief Test error chaining and propagation
 */
void test_error_chaining(void) {
    /* Set initial error */
    error_context_set(test_context, ERROR_FILE_NOT_FOUND, "Original error");
    
    /* Chain another error */
    error_context_chain(test_context, ERROR_INVALID_FORMAT, "Format error caused by file issue");
    
    TEST_ASSERT_TRUE(error_context_has_error(test_context));
    TEST_ASSERT_EQUAL_INT(ERROR_INVALID_FORMAT, error_context_get_code(test_context));
    
    /* Check if chain information is available */
    const char* chain = error_context_get_chain(test_context);
    TEST_ASSERT_NOT_NULL(chain);
    TEST_ASSERT_TRUE(strstr(chain, "Original error") != NULL);
    TEST_ASSERT_TRUE(strstr(chain, "Format error") != NULL);
}

/**
 * @brief Test error severity levels
 */
void test_error_severity(void) {
    /* Test different severity levels */
    error_context_set_with_severity(test_context, ERROR_INVALID_PARAMETER, 
                                   ERROR_SEVERITY_WARNING, "Warning message");
    
    TEST_ASSERT_EQUAL_INT(ERROR_SEVERITY_WARNING, error_context_get_severity(test_context));
    
    error_context_set_with_severity(test_context, ERROR_OUT_OF_MEMORY,
                                   ERROR_SEVERITY_CRITICAL, "Critical error");
    
    TEST_ASSERT_EQUAL_INT(ERROR_SEVERITY_CRITICAL, error_context_get_severity(test_context));
    
    /* Test severity validation */
    TEST_ASSERT_TRUE(error_severity_is_valid(ERROR_SEVERITY_INFO));
    TEST_ASSERT_TRUE(error_severity_is_valid(ERROR_SEVERITY_WARNING));
    TEST_ASSERT_TRUE(error_severity_is_valid(ERROR_SEVERITY_ERROR));
    TEST_ASSERT_TRUE(error_severity_is_valid(ERROR_SEVERITY_CRITICAL));
    TEST_ASSERT_FALSE(error_severity_is_valid((error_severity_t)255));
}

/**
 * @brief Test error callback functionality
 */
void test_error_callback(void) {
    /* Error callback state */
    static bool callback_called = false;
    static error_code_t callback_code = ERROR_SUCCESS;
    static char callback_message[256];
    
    /* Error callback function */
    auto error_callback = [](error_code_t code, const char* message, void* user_data) {
        (void)user_data; /* Unused */
        callback_called = true;
        callback_code = code;
        if (message != NULL) {
            strncpy(callback_message, message, sizeof(callback_message) - 1);
            callback_message[sizeof(callback_message) - 1] = '\0';
        }
    };
    
    /* Reset callback state */
    callback_called = false;
    callback_code = ERROR_SUCCESS;
    memset(callback_message, 0, sizeof(callback_message));
    
    /* Set error callback */
    error_context_set_callback(test_context, error_callback, NULL);
    
    /* Trigger error */
    error_context_set(test_context, ERROR_INVALID_FORMAT, "Callback test message");
    
    /* Verify callback was called */
    TEST_ASSERT_TRUE(callback_called);
    TEST_ASSERT_EQUAL_INT(ERROR_INVALID_FORMAT, callback_code);
    TEST_ASSERT_EQUAL_STRING("Callback test message", callback_message);
}

/**
 * @brief Test system error integration
 */
void test_system_error_integration(void) {
    /* Simulate system error */
    errno = ENOENT; /* File not found */
    error_context_set_from_errno(test_context, "Failed to open file");
    
    TEST_ASSERT_TRUE(error_context_has_error(test_context));
    TEST_ASSERT_EQUAL_INT(ERROR_FILE_NOT_FOUND, error_context_get_code(test_context));
    
    const char* message = error_context_get_message(test_context);
    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_TRUE(strstr(message, "Failed to open file") != NULL);
    
    /* Test with different errno values */
    errno = ENOMEM; /* Out of memory */
    error_context_set_from_errno(test_context, "Memory allocation failed");
    
    TEST_ASSERT_EQUAL_INT(ERROR_OUT_OF_MEMORY, error_context_get_code(test_context));
    
    /* Reset errno */
    errno = 0;
}

/**
 * @brief Test error context thread safety (basic)
 */
void test_error_context_thread_safety(void) {
    /* Test multiple contexts don't interfere */
    error_context_t* ctx1 = error_context_create();
    error_context_t* ctx2 = error_context_create();
    
    TEST_ASSERT_NOT_NULL(ctx1);
    TEST_ASSERT_NOT_NULL(ctx2);
    
    /* Set different errors in each context */
    error_context_set(ctx1, ERROR_INVALID_PARAMETER, "Error in context 1");
    error_context_set(ctx2, ERROR_OUT_OF_MEMORY, "Error in context 2");
    
    /* Verify contexts are independent */
    TEST_ASSERT_EQUAL_INT(ERROR_INVALID_PARAMETER, error_context_get_code(ctx1));
    TEST_ASSERT_EQUAL_INT(ERROR_OUT_OF_MEMORY, error_context_get_code(ctx2));
    
    const char* msg1 = error_context_get_message(ctx1);
    const char* msg2 = error_context_get_message(ctx2);
    
    TEST_ASSERT_EQUAL_STRING("Error in context 1", msg1);
    TEST_ASSERT_EQUAL_STRING("Error in context 2", msg2);
    
    error_context_destroy(ctx1);
    error_context_destroy(ctx2);
}

/**
 * @brief Test error string conversion
 */
void test_error_string_conversion(void) {
    /* Test error code to string conversion */
    const char* str = error_code_to_string(ERROR_SUCCESS);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_EQUAL_STRING("Success", str);
    
    str = error_code_to_string(ERROR_INVALID_PARAMETER);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_EQUAL_STRING("Invalid parameter", str);
    
    str = error_code_to_string(ERROR_OUT_OF_MEMORY);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_EQUAL_STRING("Out of memory", str);
    
    /* Test invalid error code */
    str = error_code_to_string((error_code_t)999);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_EQUAL_STRING("Unknown error", str);
    
    /* Test severity to string conversion */
    str = error_severity_to_string(ERROR_SEVERITY_INFO);
    TEST_ASSERT_EQUAL_STRING("INFO", str);
    
    str = error_severity_to_string(ERROR_SEVERITY_WARNING);
    TEST_ASSERT_EQUAL_STRING("WARNING", str);
    
    str = error_severity_to_string(ERROR_SEVERITY_ERROR);
    TEST_ASSERT_EQUAL_STRING("ERROR", str);
    
    str = error_severity_to_string(ERROR_SEVERITY_CRITICAL);
    TEST_ASSERT_EQUAL_STRING("CRITICAL", str);
}

/**
 * @brief Test error context with NULL parameters
 */
void test_error_null_parameters(void) {
    /* Test NULL context operations */
    TEST_ASSERT_FALSE(error_context_has_error(NULL));
    TEST_ASSERT_EQUAL_INT(ERROR_SUCCESS, error_context_get_code(NULL));
    TEST_ASSERT_NULL(error_context_get_message(NULL));
    
    /* Operations should not crash with NULL context */
    error_context_set(NULL, ERROR_INVALID_PARAMETER, "Should not crash");
    error_context_clear(NULL);
    error_context_setf(NULL, ERROR_OUT_OF_MEMORY, "Format %s", "test");
    
    /* Test NULL message */
    error_context_set(test_context, ERROR_INVALID_PARAMETER, NULL);
    TEST_ASSERT_TRUE(error_context_has_error(test_context));
    TEST_ASSERT_EQUAL_INT(ERROR_INVALID_PARAMETER, error_context_get_code(test_context));
    
    const char* message = error_context_get_message(test_context);
    /* Should either be NULL or default message */
    if (message != NULL) {
        TEST_ASSERT_GREATER_THAN(0, strlen(message));
    }
}

/**
 * @brief Test error message truncation for very long messages
 */
void test_error_message_truncation(void) {
    /* Create very long error message */
    char long_message[2048];
    memset(long_message, 'A', sizeof(long_message) - 1);
    long_message[sizeof(long_message) - 1] = '\0';
    
    error_context_set(test_context, ERROR_INVALID_FORMAT, long_message);
    
    const char* retrieved = error_context_get_message(test_context);
    TEST_ASSERT_NOT_NULL(retrieved);
    
    /* Message should be truncated to reasonable size */
    size_t msg_len = strlen(retrieved);
    TEST_ASSERT_LESS_THAN(1024, msg_len); /* Reasonable maximum */
    TEST_ASSERT_GREATER_THAN(0, msg_len);
}

/**
 * @brief Test error recovery and reset
 */
void test_error_recovery(void) {
    /* Set an error */
    error_context_set(test_context, ERROR_FILE_NOT_FOUND, "File missing");
    TEST_ASSERT_TRUE(error_context_has_error(test_context));
    
    /* Test reset to specific state */
    error_context_reset(test_context);
    TEST_ASSERT_FALSE(error_context_has_error(test_context));
    TEST_ASSERT_EQUAL_INT(ERROR_SUCCESS, error_context_get_code(test_context));
    
    /* Should be able to set new error after reset */
    error_context_set(test_context, ERROR_OUT_OF_MEMORY, "New error");
    TEST_ASSERT_TRUE(error_context_has_error(test_context));
    TEST_ASSERT_EQUAL_INT(ERROR_OUT_OF_MEMORY, error_context_get_code(test_context));
}

/**
 * @brief Main test runner function
 */
int main(void) {
    UNITY_BEGIN();
    
    /* Basic functionality tests */
    RUN_TEST(test_error_context_lifecycle);
    RUN_TEST(test_error_basic_operations);
    RUN_TEST(test_error_code_validation);
    
    /* Advanced functionality tests */
    RUN_TEST(test_error_message_formatting);
    RUN_TEST(test_error_chaining);
    RUN_TEST(test_error_severity);
    RUN_TEST(test_error_callback);
    
    /* Integration tests */
    RUN_TEST(test_system_error_integration);
    RUN_TEST(test_error_context_thread_safety);
    RUN_TEST(test_error_string_conversion);
    
    /* Edge case tests */
    RUN_TEST(test_error_null_parameters);
    RUN_TEST(test_error_message_truncation);
    RUN_TEST(test_error_recovery);
    
    return UNITY_END();
}
