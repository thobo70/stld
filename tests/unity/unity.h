/* tests/unity/unity.h */
#ifndef UNITY_FRAMEWORK_H
#define UNITY_FRAMEWORK_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file unity.h
 * @brief Unity Test Framework for C99
 * @details Lightweight unit testing framework for embedded systems
 */

/* Unity configuration */
#ifndef UNITY_OUTPUT_CHAR
#define UNITY_OUTPUT_CHAR(c) putchar(c)
#endif

#ifndef UNITY_OUTPUT_FLUSH
#define UNITY_OUTPUT_FLUSH() fflush(stdout)
#endif

#ifndef UNITY_OUTPUT_START
#define UNITY_OUTPUT_START()
#endif

#ifndef UNITY_OUTPUT_COMPLETE
#define UNITY_OUTPUT_COMPLETE()
#endif

/* Unity types */
typedef enum {
    UNITY_DISPLAY_STYLE_INT = 0,
    UNITY_DISPLAY_STYLE_UINT,
    UNITY_DISPLAY_STYLE_HEX8,
    UNITY_DISPLAY_STYLE_HEX16,
    UNITY_DISPLAY_STYLE_HEX32,
    UNITY_DISPLAY_STYLE_CHAR,
    UNITY_DISPLAY_STYLE_UNKNOWN
} unity_display_style_t;

typedef enum {
    UNITY_EQUAL_TO = 0,
    UNITY_NOT_EQUAL,
    UNITY_GREATER_THAN,
    UNITY_GREATER_OR_EQUAL,
    UNITY_SMALLER_THAN,
    UNITY_SMALLER_OR_EQUAL
} unity_comparison_t;

/* Forward declarations */
typedef struct unity_internals {
    const char* TestFile;
    const char* CurrentTestName;
    uint32_t CurrentTestLineNumber;
    uint32_t NumberOfTests;
    uint32_t TestFailures;
    uint32_t TestIgnores;
    uint32_t CurrentTestFailed;
    uint32_t CurrentTestIgnored;
} unity_internals_t;

/* Unity test result structure */
typedef struct {
    uint32_t tests_run;
    uint32_t tests_passed;
    uint32_t tests_failed;
    uint32_t tests_ignored;
    const char* current_test_name;
    const char* current_test_file;
    uint32_t current_test_line;
} unity_test_results_t;

/* Unity function prototypes */
void unity_begin(const char* filename);
int unity_end(void);
void unity_print(const char* string);
void unity_print_number(const int32_t number, const unity_display_style_t style);
void unity_print_float(const float number);

/* Test assertion functions */
void unity_test_assert_equal_int(const int32_t expected,
                                const int32_t actual,
                                const char* msg,
                                const uint32_t line_number,
                                const unity_comparison_t style);

void unity_test_assert_equal_uint(const uint32_t expected,
                                 const uint32_t actual,
                                 const char* msg,
                                 const uint32_t line_number,
                                 const unity_comparison_t style);

void unity_test_assert_equal_hex8(const uint8_t expected,
                                 const uint8_t actual,
                                 const char* msg,
                                 const uint32_t line_number);

void unity_test_assert_equal_hex16(const uint16_t expected,
                                  const uint16_t actual,
                                  const char* msg,
                                  const uint32_t line_number);

void unity_test_assert_equal_hex32(const uint32_t expected,
                                  const uint32_t actual,
                                  const char* msg,
                                  const uint32_t line_number);

void unity_test_assert_equal_ptr(const void* expected,
                                const void* actual,
                                const char* msg,
                                const uint32_t line_number);

void unity_test_assert_equal_string(const char* expected,
                                   const char* actual,
                                   const char* msg,
                                   const uint32_t line_number);

void unity_test_assert_equal_memory(const void* expected,
                                   const void* actual,
                                   const uint32_t length,
                                   const char* msg,
                                   const uint32_t line_number);

void unity_test_assert_null(const void* pointer,
                           const char* msg,
                           const uint32_t line_number);

void unity_test_assert_not_null(const void* pointer,
                               const char* msg,
                               const uint32_t line_number);

void unity_test_fail(const char* msg, const uint32_t line_number);

void unity_test_ignore(const char* msg, const uint32_t line_number);

/* Test macros */
#define UNITY_BEGIN() unity_begin(__FILE__)
#define UNITY_END() unity_end()

#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            unity_test_fail("Condition failed: " #condition, __LINE__); \
        } \
    } while (0)

#define TEST_ASSERT_TRUE(condition) \
    TEST_ASSERT(condition)

#define TEST_ASSERT_FALSE(condition) \
    TEST_ASSERT(!(condition))

#define TEST_ASSERT_EQUAL_INT(expected, actual) \
    unity_test_assert_equal_int((expected), (actual), NULL, __LINE__, UNITY_EQUAL_TO)

#define TEST_ASSERT_EQUAL_UINT(expected, actual) \
    unity_test_assert_equal_uint((expected), (actual), NULL, __LINE__, UNITY_EQUAL_TO)

#define TEST_ASSERT_EQUAL_HEX8(expected, actual) \
    unity_test_assert_equal_hex8((expected), (actual), NULL, __LINE__)

#define TEST_ASSERT_EQUAL_HEX16(expected, actual) \
    unity_test_assert_equal_hex16((expected), (actual), NULL, __LINE__)

#define TEST_ASSERT_EQUAL_HEX32(expected, actual) \
    unity_test_assert_equal_hex32((expected), (actual), NULL, __LINE__)

#define TEST_ASSERT_EQUAL_PTR(expected, actual) \
    unity_test_assert_equal_ptr((expected), (actual), NULL, __LINE__)

#define TEST_ASSERT_NULL(pointer) \
    unity_test_assert_null((pointer), NULL, __LINE__)

#define TEST_ASSERT_NOT_NULL(pointer) \
    unity_test_assert_not_null((pointer), NULL, __LINE__)

#define TEST_ASSERT_EQUAL_STRING(expected, actual) \
    unity_test_assert_equal_string((expected), (actual), NULL, __LINE__)

#define TEST_ASSERT_EQUAL_MEMORY(expected, actual, length) \
    unity_test_assert_equal_memory((expected), (actual), (length), NULL, __LINE__)

#define TEST_FAIL() \
    unity_test_fail("Test failed", __LINE__)

#define TEST_FAIL_MESSAGE(message) \
    unity_test_fail((message), __LINE__)

#define TEST_IGNORE() \
    unity_test_ignore("Test ignored", __LINE__)

#define TEST_IGNORE_MESSAGE(message) \
    unity_test_ignore((message), __LINE__)

/* Comparison variants */
#define TEST_ASSERT_NOT_EQUAL_INT(expected, actual) \
    unity_test_assert_equal_int((expected), (actual), NULL, __LINE__, UNITY_NOT_EQUAL)

#define TEST_ASSERT_GREATER_THAN_INT(threshold, actual) \
    unity_test_assert_equal_int((threshold), (actual), NULL, __LINE__, UNITY_GREATER_THAN)

#define TEST_ASSERT_GREATER_OR_EQUAL_INT(threshold, actual) \
    unity_test_assert_equal_int((threshold), (actual), NULL, __LINE__, UNITY_GREATER_OR_EQUAL)

#define TEST_ASSERT_LESS_THAN_INT(threshold, actual) \
    unity_test_assert_equal_int((threshold), (actual), NULL, __LINE__, UNITY_SMALLER_THAN)

#define TEST_ASSERT_LESS_OR_EQUAL_INT(threshold, actual) \
    unity_test_assert_equal_int((threshold), (actual), NULL, __LINE__, UNITY_SMALLER_OR_EQUAL)

/* Test runner macros */
#define RUN_TEST(func) \
    do { \
        Unity.CurrentTestName = #func; \
        Unity.CurrentTestFailed = 0; \
        Unity.CurrentTestIgnored = 0; \
        Unity.NumberOfTests++; \
        unity_print("Running "); \
        unity_print(#func); \
        unity_print("..."); \
        setUp(); \
        func(); \
        tearDown(); \
        if (Unity.CurrentTestFailed) { \
            Unity.TestFailures++; \
            unity_print(" FAIL\n"); \
        } else if (Unity.CurrentTestIgnored) { \
            Unity.TestIgnores++; \
            unity_print(" IGNORE\n"); \
        } else { \
            unity_print(" PASS\n"); \
        } \
    } while (0)

/* Test fixture function prototypes */
void setUp(void);
void tearDown(void);

/* Global test results */
extern unity_test_results_t unity_test_results;
extern unity_internals_t Unity;

#ifdef __cplusplus
}
#endif

#endif /* UNITY_FRAMEWORK_H */
