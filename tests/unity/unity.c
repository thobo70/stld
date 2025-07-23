/* tests/unity/unity.c */
#include "unity.h"
#include "unity_internals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

/**
 * @file unity.c
 * @brief Unity Test Framework implementation
 * @details C99 compliant lightweight testing framework
 */

/* Global Unity state */
unity_internals_t Unity;
unity_test_results_t unity_test_results;

/* Weak implementations for setUp/tearDown */
__attribute__((weak)) void setUp(void) {
    /* Default empty implementation */
}

__attribute__((weak)) void tearDown(void) {
    /* Default empty implementation */
}

void unity_begin(const char* filename) {
    Unity.TestFile = filename;
    Unity.CurrentTestName = NULL;
    Unity.CurrentTestLineNumber = 0;
    Unity.NumberOfTests = 0;
    Unity.TestFailures = 0;
    Unity.TestIgnores = 0;
    Unity.CurrentTestFailed = 0;
    Unity.CurrentTestIgnored = 0;
    
    unity_test_results.tests_run = 0;
    unity_test_results.tests_passed = 0;
    unity_test_results.tests_failed = 0;
    unity_test_results.tests_ignored = 0;
    unity_test_results.current_test_name = NULL;
    unity_test_results.current_test_file = filename;
    unity_test_results.current_test_line = 0;
    
    UNITY_OUTPUT_START();
    unity_print("Unity Test Framework\n");
    unity_print("File: ");
    unity_print(filename);
    unity_print("\n\n");
}

int unity_end(void) {
    UNITY_OUTPUT_COMPLETE();
    
    unity_print("\n-----------------------\n");
    unity_print_number((int32_t)Unity.NumberOfTests, UNITY_DISPLAY_STYLE_UINT);
    unity_print(" Tests ");
    unity_print_number((int32_t)Unity.TestFailures, UNITY_DISPLAY_STYLE_UINT);
    unity_print(" Failures ");
    unity_print_number((int32_t)Unity.TestIgnores, UNITY_DISPLAY_STYLE_UINT);
    unity_print(" Ignored\n");
    
    if (Unity.TestFailures == 0 && Unity.TestIgnores == 0) {
        unity_print("OK\n");
    } else {
        unity_print("FAIL\n");
    }
    
    return (int)Unity.TestFailures;
}

void unity_print(const char* string) {
    const char* pch = string;
    if (pch != NULL) {
        while (*pch) {
            UNITY_OUTPUT_CHAR(*pch);
            pch++;
        }
    }
}

void unity_print_number(const int32_t number, const unity_display_style_t style) {
    char buffer[16];
    
    switch (style) {
        case UNITY_DISPLAY_STYLE_INT:
            sprintf(buffer, "%d", (int)number);
            break;
        case UNITY_DISPLAY_STYLE_UINT:
            sprintf(buffer, "%u", (unsigned int)number);
            break;
        case UNITY_DISPLAY_STYLE_HEX8:
            sprintf(buffer, "0x%02X", (unsigned int)(number & 0xFF));
            break;
        case UNITY_DISPLAY_STYLE_HEX16:
            sprintf(buffer, "0x%04X", (unsigned int)(number & 0xFFFF));
            break;
        case UNITY_DISPLAY_STYLE_HEX32:
            sprintf(buffer, "0x%08X", (unsigned int)number);
            break;
        case UNITY_DISPLAY_STYLE_CHAR:
            sprintf(buffer, "'%c'", (char)number);
            break;
        default:
            sprintf(buffer, "%d", (int)number);
            break;
    }
    
    unity_print(buffer);
}

void unity_print_float(const float number) {
    char buffer[32];
    sprintf(buffer, "%.6f", number);
    unity_print(buffer);
}

void unity_test_result_for_numbers(const int32_t expected,
                                         const int32_t actual,
                                         const unity_comparison_t style) {
    bool result = false;
    
    switch (style) {
        case UNITY_EQUAL_TO:
            result = (expected == actual);
            break;
        case UNITY_NOT_EQUAL:
            result = (expected != actual);
            break;
        case UNITY_GREATER_THAN:
            result = (actual > expected);
            break;
        case UNITY_GREATER_OR_EQUAL:
            result = (actual >= expected);
            break;
        case UNITY_SMALLER_THAN:
            result = (actual < expected);
            break;
        case UNITY_SMALLER_OR_EQUAL:
            result = (actual <= expected);
            break;
    }
    
    if (!result) {
        Unity.CurrentTestFailed = 1;
        unity_print("Expected ");
        unity_print_number(expected, UNITY_DISPLAY_STYLE_INT);
        unity_print(" Was ");
        unity_print_number(actual, UNITY_DISPLAY_STYLE_INT);
    }
}

void unity_test_assert_equal_int(const int32_t expected,
                                const int32_t actual,
                                const char* msg,
                                const uint32_t line_number,
                                const unity_comparison_t style) {
    Unity.CurrentTestLineNumber = line_number;
    unity_test_result_for_numbers(expected, actual, style);
    
    if (Unity.CurrentTestFailed) {
        if (msg != NULL) {
            unity_print(" - ");
            unity_print(msg);
        }
        unity_print("\n");
    }
}

void unity_test_assert_equal_uint(const uint32_t expected,
                                 const uint32_t actual,
                                 const char* msg,
                                 const uint32_t line_number,
                                 const unity_comparison_t style) {
    unity_test_assert_equal_int((int32_t)expected, (int32_t)actual, msg, line_number, style);
}

void unity_test_assert_equal_hex8(const uint8_t expected,
                                 const uint8_t actual,
                                 const char* msg,
                                 const uint32_t line_number) {
    Unity.CurrentTestLineNumber = line_number;
    
    if (expected != actual) {
        Unity.CurrentTestFailed = 1;
        unity_print("Expected ");
        unity_print_number(expected, UNITY_DISPLAY_STYLE_HEX8);
        unity_print(" Was ");
        unity_print_number(actual, UNITY_DISPLAY_STYLE_HEX8);
        
        if (msg != NULL) {
            unity_print(" - ");
            unity_print(msg);
        }
        unity_print("\n");
    }
}

void unity_test_assert_equal_hex16(const uint16_t expected,
                                  const uint16_t actual,
                                  const char* msg,
                                  const uint32_t line_number) {
    Unity.CurrentTestLineNumber = line_number;
    
    if (expected != actual) {
        Unity.CurrentTestFailed = 1;
        unity_print("Expected ");
        unity_print_number(expected, UNITY_DISPLAY_STYLE_HEX16);
        unity_print(" Was ");
        unity_print_number(actual, UNITY_DISPLAY_STYLE_HEX16);
        
        if (msg != NULL) {
            unity_print(" - ");
            unity_print(msg);
        }
        unity_print("\n");
    }
}

void unity_test_assert_equal_hex32(const uint32_t expected,
                                  const uint32_t actual,
                                  const char* msg,
                                  const uint32_t line_number) {
    Unity.CurrentTestLineNumber = line_number;
    
    if (expected != actual) {
        Unity.CurrentTestFailed = 1;
        unity_print("Expected ");
        unity_print_number((int32_t)expected, UNITY_DISPLAY_STYLE_HEX32);
        unity_print(" Was ");
        unity_print_number((int32_t)actual, UNITY_DISPLAY_STYLE_HEX32);
        
        if (msg != NULL) {
            unity_print(" - ");
            unity_print(msg);
        }
        unity_print("\n");
    }
}

void unity_test_assert_equal_ptr(const void* expected,
                                const void* actual,
                                const char* msg,
                                const uint32_t line_number) {
    Unity.CurrentTestLineNumber = line_number;
    
    if (expected != actual) {
        Unity.CurrentTestFailed = 1;
        unity_print("Expected pointer ");
        unity_print_number((int32_t)(uintptr_t)expected, UNITY_DISPLAY_STYLE_HEX32);
        unity_print(" Was ");
        unity_print_number((int32_t)(uintptr_t)actual, UNITY_DISPLAY_STYLE_HEX32);
        
        if (msg != NULL) {
            unity_print(" - ");
            unity_print(msg);
        }
        unity_print("\n");
    }
}

void unity_test_assert_equal_string(const char* expected,
                                   const char* actual,
                                   const char* msg,
                                   const uint32_t line_number) {
    Unity.CurrentTestLineNumber = line_number;
    
    if ((expected == NULL && actual != NULL) ||
        (expected != NULL && actual == NULL) ||
        (expected != NULL && actual != NULL && strcmp(expected, actual) != 0)) {
        
        Unity.CurrentTestFailed = 1;
        unity_print("Expected \"");
        unity_print(expected ? expected : "NULL");
        unity_print("\" Was \"");
        unity_print(actual ? actual : "NULL");
        unity_print("\"");
        
        if (msg != NULL) {
            unity_print(" - ");
            unity_print(msg);
        }
        unity_print("\n");
    }
}

void unity_test_assert_equal_memory(const void* expected,
                                   const void* actual,
                                   const uint32_t length,
                                   const char* msg,
                                   const uint32_t line_number) {
    Unity.CurrentTestLineNumber = line_number;
    
    if ((expected == NULL && actual != NULL) ||
        (expected != NULL && actual == NULL) ||
        (expected != NULL && actual != NULL && memcmp(expected, actual, length) != 0)) {
        
        Unity.CurrentTestFailed = 1;
        unity_print("Expected memory differs from actual");
        
        if (msg != NULL) {
            unity_print(" - ");
            unity_print(msg);
        }
        unity_print("\n");
    }
}

void unity_test_assert_null(const void* pointer,
                           const char* msg,
                           const uint32_t line_number) {
    Unity.CurrentTestLineNumber = line_number;
    
    if (pointer != NULL) {
        Unity.CurrentTestFailed = 1;
        unity_print("Expected NULL Was ");
        unity_print_number((int32_t)(uintptr_t)pointer, UNITY_DISPLAY_STYLE_HEX32);
        
        if (msg != NULL) {
            unity_print(" - ");
            unity_print(msg);
        }
        unity_print("\n");
    }
}

void unity_test_assert_not_null(const void* pointer,
                               const char* msg,
                               const uint32_t line_number) {
    Unity.CurrentTestLineNumber = line_number;
    
    if (pointer == NULL) {
        Unity.CurrentTestFailed = 1;
        unity_print("Expected non-NULL pointer Was NULL");
        
        if (msg != NULL) {
            unity_print(" - ");
            unity_print(msg);
        }
        unity_print("\n");
    }
}

void unity_test_fail(const char* msg, const uint32_t line_number) {
    Unity.CurrentTestLineNumber = line_number;
    Unity.CurrentTestFailed = 1;
    
    unity_print("FAIL");
    if (msg != NULL) {
        unity_print(" - ");
        unity_print(msg);
    }
    unity_print("\n");
}

void unity_test_ignore(const char* msg, const uint32_t line_number) {
    Unity.CurrentTestLineNumber = line_number;
    Unity.CurrentTestIgnored = 1;
    
    unity_print("IGNORE");
    if (msg != NULL) {
        unity_print(" - ");
        unity_print(msg);
    }
    unity_print("\n");
}
