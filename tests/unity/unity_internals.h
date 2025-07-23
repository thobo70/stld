/* tests/unity/unity_internals.h */
#ifndef UNITY_INTERNALS_H
#define UNITY_INTERNALS_H

#include "unity.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file unity_internals.h
 * @brief Internal definitions for Unity framework
 * @details Private functions and data structures for Unity implementation
 */

/* Internal configuration */
#define UNITY_COUNTER_TYPE uint32_t
#define UNITY_DISPLAY_RANGE_INT 32
#define UNITY_DISPLAY_RANGE_UINT 32
#define UNITY_DISPLAY_RANGE_HEX 32

/* Internal state tracking */
/* Unity internal structure - defined in unity.h */

/* Internal helper functions */
void unity_add_number(const int32_t number, const unity_display_style_t style);
void unity_print_expected_and_actual_strings(const char* expected, const char* actual);
void unity_print_expected_and_actual_strings_len(const char* expected,
                                                 const char* actual,
                                                 const uint32_t length);

/* Internal assertion helpers */
void unity_test_result_for_numbers(const int32_t expected,
                                  const int32_t actual,
                                  const unity_comparison_t style);

void unity_test_result_for_pointers(const void* expected, const void* actual);

void unity_test_result_for_strings(const char* expected,
                                  const char* actual,
                                  const uint32_t length);

/* Internal test control */
void unity_set_test_file(const char* filename);
void unity_test_conclude(void);

#ifdef __cplusplus
}
#endif

#endif /* UNITY_INTERNALS_H */
