/**
 * @file unity_config.h
 * @brief Unity testing framework configuration for STLD/STAR project
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * Configuration file for Unity testing framework with optimizations
 * for embedded C99 development and comprehensive test reporting.
 */

#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H

/* Unity feature configuration */
#define UNITY_INCLUDE_64
#define UNITY_INCLUDE_FLOAT
#define UNITY_INCLUDE_DOUBLE
#define UNITY_SUPPORT_64

/* Unity output configuration */
#define UNITY_OUTPUT_CHAR(c)         putchar(c)
#define UNITY_OUTPUT_FLUSH()         fflush(stdout)
#define UNITY_OUTPUT_START()         /* No special start */
#define UNITY_OUTPUT_COMPLETE()      /* No special complete */

/* Unity color output for better readability */
#ifdef UNITY_OUTPUT_COLOR
#define UNITY_OUTPUT_COLOR_ASSERT    "\033[31m"  /* Red */
#define UNITY_OUTPUT_COLOR_IGNORE    "\033[33m"  /* Yellow */
#define UNITY_OUTPUT_COLOR_PASS      "\033[32m"  /* Green */
#define UNITY_OUTPUT_COLOR_NORMAL    "\033[0m"   /* Normal */
#endif

/* Memory allocation configuration */
#define UNITY_EXCLUDE_STDLIB_MALLOC

/* Test fixture configuration */
#define UNITY_FIXTURE_MAX_TESTS      1000

/* Verbose output configuration */
#ifdef UNITY_VERBOSE
#define UNITY_PRINT_EOL()            UNITY_OUTPUT_CHAR('\n')
#else
#define UNITY_PRINT_EOL()            /* Minimal output */
#endif

/* Custom assertion macros for STLD/STAR specific types */
#define TEST_ASSERT_MEMORY_POOL_VALID(pool) \
    do { \
        TEST_ASSERT_NOT_NULL(pool); \
        TEST_ASSERT_GREATER_THAN(0, memory_pool_get_size(pool)); \
        TEST_ASSERT_GREATER_OR_EQUAL(0, memory_pool_get_used(pool)); \
        TEST_ASSERT_LESS_OR_EQUAL(memory_pool_get_size(pool), memory_pool_get_used(pool)); \
    } while(0)

#define TEST_ASSERT_ERROR_CONTEXT_VALID(ctx) \
    do { \
        TEST_ASSERT_NOT_NULL(ctx); \
    } while(0)

#define TEST_ASSERT_SMOF_HEADER_VALID(header) \
    do { \
        TEST_ASSERT_NOT_NULL(header); \
        TEST_ASSERT_EQUAL_HEX32(SMOF_MAGIC, (header)->magic); \
        TEST_ASSERT_GREATER_THAN(0, (header)->version); \
    } while(0)

#define TEST_ASSERT_SYMBOL_VALID(symbol) \
    do { \
        TEST_ASSERT_NOT_NULL(symbol); \
        TEST_ASSERT_NOT_NULL((symbol)->name); \
        TEST_ASSERT_GREATER_THAN(0, strlen((symbol)->name)); \
    } while(0)

#define TEST_ASSERT_SECTION_VALID(section) \
    do { \
        TEST_ASSERT_NOT_NULL(section); \
        TEST_ASSERT_NOT_NULL((section)->name); \
        TEST_ASSERT_GREATER_OR_EQUAL(0, (section)->size); \
    } while(0)

#define TEST_ASSERT_ARCHIVE_VALID(archive) \
    do { \
        TEST_ASSERT_NOT_NULL(archive); \
        TEST_ASSERT_GREATER_OR_EQUAL(0, archive_get_file_count(archive)); \
    } while(0)

/* Performance testing macros */
#include <time.h>

#define TEST_PERFORMANCE_START() \
    clock_t _perf_start = clock()

#define TEST_PERFORMANCE_END(max_ms) \
    do { \
        clock_t _perf_end = clock(); \
        double _perf_elapsed = ((double)(_perf_end - _perf_start)) / CLOCKS_PER_SEC * 1000.0; \
        TEST_ASSERT_LESS_THAN_DOUBLE((double)(max_ms), _perf_elapsed); \
    } while(0)

/* Memory testing helpers */
#define TEST_MEMORY_LEAK_START() \
    size_t _initial_used = memory_pool_get_used(test_memory_pool)

#define TEST_MEMORY_LEAK_END() \
    do { \
        size_t _final_used = memory_pool_get_used(test_memory_pool); \
        TEST_ASSERT_EQUAL_size_t(_initial_used, _final_used); \
    } while(0)

/* String comparison with null safety */
#define TEST_ASSERT_EQUAL_STRING_OR_NULL(expected, actual) \
    do { \
        if ((expected) == NULL && (actual) == NULL) { \
            /* Both null - OK */ \
        } else if ((expected) == NULL || (actual) == NULL) { \
            TEST_FAIL_MESSAGE("One string is NULL, the other is not"); \
        } else { \
            TEST_ASSERT_EQUAL_STRING((expected), (actual)); \
        } \
    } while(0)

/* Array comparison helper */
#define TEST_ASSERT_EQUAL_UINT8_ARRAY_OR_NULL(expected, actual, length) \
    do { \
        if ((expected) == NULL && (actual) == NULL) { \
            /* Both null - OK */ \
        } else if ((expected) == NULL || (actual) == NULL) { \
            TEST_FAIL_MESSAGE("One array is NULL, the other is not"); \
        } else { \
            TEST_ASSERT_EQUAL_UINT8_ARRAY((expected), (actual), (length)); \
        } \
    } while(0)

/* Floating point comparison with tolerance */
#define TEST_ASSERT_FLOAT_WITHIN_PERCENT(percent, expected, actual) \
    do { \
        float _tolerance = (expected) * (percent) / 100.0f; \
        TEST_ASSERT_FLOAT_WITHIN(_tolerance, (expected), (actual)); \
    } while(0)

/* Range checking macros */
#define TEST_ASSERT_IN_RANGE(min, max, actual) \
    do { \
        TEST_ASSERT_GREATER_OR_EQUAL((min), (actual)); \
        TEST_ASSERT_LESS_OR_EQUAL((max), (actual)); \
    } while(0)

#define TEST_ASSERT_IN_RANGE_size_t(min, max, actual) \
    do { \
        TEST_ASSERT_GREATER_OR_EQUAL_size_t((min), (actual)); \
        TEST_ASSERT_LESS_OR_EQUAL_size_t((max), (actual)); \
    } while(0)

/* Bit manipulation testing */
#define TEST_ASSERT_BIT_SET(bit, value) \
    TEST_ASSERT_TRUE(((value) & (1U << (bit))) != 0)

#define TEST_ASSERT_BIT_CLEAR(bit, value) \
    TEST_ASSERT_TRUE(((value) & (1U << (bit))) == 0)

#define TEST_ASSERT_BITS_SET(mask, value) \
    TEST_ASSERT_EQUAL_HEX32((mask), ((value) & (mask)))

/* Endianness testing */
#define TEST_ASSERT_UINT16_BE(expected, buffer) \
    do { \
        uint16_t _actual = ((uint16_t)(buffer)[0] << 8) | (buffer)[1]; \
        TEST_ASSERT_EQUAL_HEX16((expected), _actual); \
    } while(0)

#define TEST_ASSERT_UINT32_BE(expected, buffer) \
    do { \
        uint32_t _actual = ((uint32_t)(buffer)[0] << 24) | \
                          ((uint32_t)(buffer)[1] << 16) | \
                          ((uint32_t)(buffer)[2] << 8) | \
                          (buffer)[3]; \
        TEST_ASSERT_EQUAL_HEX32((expected), _actual); \
    } while(0)

#define TEST_ASSERT_UINT16_LE(expected, buffer) \
    do { \
        uint16_t _actual = (buffer)[0] | ((uint16_t)(buffer)[1] << 8); \
        TEST_ASSERT_EQUAL_HEX16((expected), _actual); \
    } while(0)

#define TEST_ASSERT_UINT32_LE(expected, buffer) \
    do { \
        uint32_t _actual = (buffer)[0] | \
                          ((uint32_t)(buffer)[1] << 8) | \
                          ((uint32_t)(buffer)[2] << 16) | \
                          ((uint32_t)(buffer)[3] << 24); \
        TEST_ASSERT_EQUAL_HEX32((expected), _actual); \
    } while(0)

/* File system testing helpers (when applicable) */
#ifdef TEST_FILE_SYSTEM_AVAILABLE
#include <sys/stat.h>

#define TEST_ASSERT_FILE_EXISTS(filepath) \
    do { \
        struct stat _st; \
        TEST_ASSERT_EQUAL_INT(0, stat((filepath), &_st)); \
        TEST_ASSERT_TRUE(S_ISREG(_st.st_mode)); \
    } while(0)

#define TEST_ASSERT_FILE_SIZE(expected_size, filepath) \
    do { \
        struct stat _st; \
        TEST_ASSERT_EQUAL_INT(0, stat((filepath), &_st)); \
        TEST_ASSERT_EQUAL_size_t((expected_size), (size_t)_st.st_size); \
    } while(0)
#endif

/* Test group management */
#define TEST_GROUP(name) \
    static const char* TEST_GROUP_NAME = name; \
    static void print_test_group_banner(void) { \
        printf("\n=== %s Tests ===\n", TEST_GROUP_NAME); \
    }

#define TEST_CASE(name) \
    void test_##name(void)

/* Test timeout configuration (in seconds) */
#define UNITY_TEST_TIMEOUT_SECONDS   30

/* Maximum test name length */
#define UNITY_MAX_TEST_NAME_LEN      128

/* Test statistics tracking */
typedef struct {
    size_t tests_run;
    size_t tests_passed;
    size_t tests_failed;
    size_t tests_ignored;
    double total_time;
} test_stats_t;

/* Test result categorization */
typedef enum {
    TEST_RESULT_PASS = 0,
    TEST_RESULT_FAIL = 1,
    TEST_RESULT_IGNORE = 2,
    TEST_RESULT_TIMEOUT = 3,
    TEST_RESULT_CRASH = 4
} test_result_t;

/* Custom test runner configuration */
#ifdef UNITY_CUSTOM_RUNNER
extern void UnityCustomTestRunner(void);
#define UNITY_USE_CUSTOM_RUNNER UnityCustomTestRunner
#endif

/* Integration with continuous integration systems */
#ifdef CI_BUILD
#define UNITY_OUTPUT_FOR_CI
#define UNITY_EXCLUDE_DETAILS
#endif

/* Memory pool alignment for testing */
#ifndef TEST_MEMORY_ALIGNMENT
#define TEST_MEMORY_ALIGNMENT        8
#endif

/* Test data size limits */
#ifndef TEST_MAX_DATA_SIZE
#define TEST_MAX_DATA_SIZE           (64 * 1024)  /* 64KB */
#endif

#ifndef TEST_MAX_STRING_LENGTH
#define TEST_MAX_STRING_LENGTH       256
#endif

/* Debugging support */
#ifdef DEBUG_TESTS
#define TEST_DEBUG_PRINT(fmt, ...) \
    printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define TEST_DEBUG_PRINT(fmt, ...) \
    /* No debug output in release builds */
#endif

#endif /* UNITY_CONFIG_H */
