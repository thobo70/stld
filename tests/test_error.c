/* tests/test_error.c */
#include "unity.h"
#include "error.h"
#include <stdio.h>
#include <string.h>

/**
 * @file test_error.c
 * @brief Unit tests for error handling system
 * @details Tests error code translation, error context, and error reporting
 */

void setUp(void) {
    /* Setup before each test */
    smof_error_clear();
}

void tearDown(void) {
    /* Cleanup after each test */
    smof_error_clear();
}

void test_error_code_to_string(void) {
    TEST_ASSERT_EQUAL_STRING("Success", smof_error_to_string(SMOF_SUCCESS));
    TEST_ASSERT_EQUAL_STRING("Out of memory", smof_error_to_string(SMOF_ERROR_OUT_OF_MEMORY));
    TEST_ASSERT_EQUAL_STRING("Invalid argument", smof_error_to_string(SMOF_ERROR_INVALID_ARGUMENT));
    TEST_ASSERT_EQUAL_STRING("Invalid magic number", smof_error_to_string(SMOF_ERROR_INVALID_MAGIC));
    TEST_ASSERT_EQUAL_STRING("Unsupported version", smof_error_to_string(SMOF_ERROR_UNSUPPORTED_VERSION));
    TEST_ASSERT_EQUAL_STRING("Insufficient data", smof_error_to_string(SMOF_ERROR_INSUFFICIENT_DATA));
    TEST_ASSERT_EQUAL_STRING("Section not found", smof_error_to_string(SMOF_ERROR_SECTION_NOT_FOUND));
    TEST_ASSERT_EQUAL_STRING("Symbol not found", smof_error_to_string(SMOF_ERROR_SYMBOL_NOT_FOUND));
    TEST_ASSERT_EQUAL_STRING("Invalid relocation", smof_error_to_string(SMOF_ERROR_INVALID_RELOCATION));
    TEST_ASSERT_EQUAL_STRING("File I/O error", smof_error_to_string(SMOF_ERROR_FILE_IO));
    TEST_ASSERT_EQUAL_STRING("Unknown error", smof_error_to_string(999)); /* Invalid code */
}

void test_error_set_and_get(void) {
    /* Initially no error */
    TEST_ASSERT_EQUAL_INT(SMOF_SUCCESS, smof_error_get_last());
    TEST_ASSERT_NULL(smof_error_get_message());
    
    /* Set an error */
    smof_error_set(SMOF_ERROR_OUT_OF_MEMORY, "Failed to allocate buffer");
    
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_OUT_OF_MEMORY, smof_error_get_last());
    TEST_ASSERT_EQUAL_STRING("Failed to allocate buffer", smof_error_get_message());
}

void test_error_set_null_message(void) {
    smof_error_set(SMOF_ERROR_INVALID_ARGUMENT, NULL);
    
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_INVALID_ARGUMENT, smof_error_get_last());
    TEST_ASSERT_NULL(smof_error_get_message());
}

void test_error_set_empty_message(void) {
    smof_error_set(SMOF_ERROR_FILE_IO, "");
    
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_FILE_IO, smof_error_get_last());
    TEST_ASSERT_EQUAL_STRING("", smof_error_get_message());
}

void test_error_set_long_message(void) {
    char long_message[300];
    memset(long_message, 'A', sizeof(long_message) - 1);
    long_message[sizeof(long_message) - 1] = '\0';
    
    smof_error_set(SMOF_ERROR_INVALID_MAGIC, long_message);
    
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_INVALID_MAGIC, smof_error_get_last());
    
    const char* stored_message = smof_error_get_message();
    TEST_ASSERT_NOT_NULL(stored_message);
    TEST_ASSERT_TRUE(strlen(stored_message) < 300); /* Should be truncated */
}

void test_error_clear(void) {
    /* Set an error first */
    smof_error_set(SMOF_ERROR_SYMBOL_NOT_FOUND, "Symbol 'main' not found");
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_SYMBOL_NOT_FOUND, smof_error_get_last());
    
    /* Clear the error */
    smof_error_clear();
    
    TEST_ASSERT_EQUAL_INT(SMOF_SUCCESS, smof_error_get_last());
    TEST_ASSERT_NULL(smof_error_get_message());
}

void test_error_overwrite(void) {
    /* Set first error */
    smof_error_set(SMOF_ERROR_OUT_OF_MEMORY, "First error");
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_OUT_OF_MEMORY, smof_error_get_last());
    TEST_ASSERT_EQUAL_STRING("First error", smof_error_get_message());
    
    /* Set second error (should overwrite) */
    smof_error_set(SMOF_ERROR_FILE_IO, "Second error");
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_FILE_IO, smof_error_get_last());
    TEST_ASSERT_EQUAL_STRING("Second error", smof_error_get_message());
}

void test_error_format_message(void) {
    char formatted[256];
    
    smof_error_format_message(SMOF_ERROR_SECTION_NOT_FOUND, "test.o", 42, formatted, sizeof(formatted));
    
    /* Should contain error string, filename, and line number */
    TEST_ASSERT_TRUE(strstr(formatted, "Section not found") != NULL);
    TEST_ASSERT_TRUE(strstr(formatted, "test.o") != NULL);
    TEST_ASSERT_TRUE(strstr(formatted, "42") != NULL);
}

void test_error_format_message_null_filename(void) {
    char formatted[256];
    
    smof_error_format_message(SMOF_ERROR_INVALID_ARGUMENT, NULL, 0, formatted, sizeof(formatted));
    
    TEST_ASSERT_TRUE(strstr(formatted, "Invalid argument") != NULL);
    /* Should handle NULL filename gracefully */
}

void test_error_format_message_small_buffer(void) {
    char small_buffer[10];
    
    smof_error_format_message(SMOF_ERROR_OUT_OF_MEMORY, "very_long_filename.o", 12345, 
                             small_buffer, sizeof(small_buffer));
    
    /* Should not crash and should be null-terminated */
    TEST_ASSERT_EQUAL_CHAR('\0', small_buffer[sizeof(small_buffer) - 1]);
}

void test_error_print(void) {
    /* This test mainly ensures the function doesn't crash */
    smof_error_set(SMOF_ERROR_INVALID_RELOCATION, "Bad relocation type");
    
    /* Redirect stderr to /dev/null for testing */
    FILE* old_stderr = stderr;
    stderr = fopen("/dev/null", "w");
    
    smof_error_print("test_file.o", 100);
    
    fclose(stderr);
    stderr = old_stderr;
    
    /* If we get here, the function didn't crash */
    TEST_ASSERT_TRUE(true);
}

void test_error_has_error(void) {
    /* Initially no error */
    TEST_ASSERT_FALSE(smof_error_has_error());
    
    /* Set an error */
    smof_error_set(SMOF_ERROR_UNSUPPORTED_VERSION, "Version 2.0 not supported");
    TEST_ASSERT_TRUE(smof_error_has_error());
    
    /* Clear error */
    smof_error_clear();
    TEST_ASSERT_FALSE(smof_error_has_error());
}

void test_error_thread_safety_simulation(void) {
    /* Simulate rapid error setting/clearing as might happen in multithreaded code */
    for (int i = 0; i < 100; i++) {
        smof_error_set(SMOF_ERROR_OUT_OF_MEMORY, "Memory error");
        TEST_ASSERT_TRUE(smof_error_has_error());
        smof_error_clear();
        TEST_ASSERT_FALSE(smof_error_has_error());
    }
}

int test_error_main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_error_code_to_string);
    RUN_TEST(test_error_set_and_get);
    RUN_TEST(test_error_set_null_message);
    RUN_TEST(test_error_set_empty_message);
    RUN_TEST(test_error_set_long_message);
    RUN_TEST(test_error_clear);
    RUN_TEST(test_error_overwrite);
    RUN_TEST(test_error_format_message);
    RUN_TEST(test_error_format_message_null_filename);
    RUN_TEST(test_error_format_message_small_buffer);
    RUN_TEST(test_error_print);
    RUN_TEST(test_error_has_error);
    RUN_TEST(test_error_thread_safety_simulation);
    
    return UNITY_END();
}

int main(void) {
    return test_error_main();
}
