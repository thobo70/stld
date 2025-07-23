/* tests/test_runner.c */
#include "unity.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @file test_runner.c
 * @brief Main test runner for all STLD tests
 * @details Runs all unit and integration tests in sequence
 */

/* External test function declarations */
extern int test_memory_main(void);
extern int test_smof_main(void);
extern int test_error_main(void);
extern int test_integration_main(void);

/* Test runner structure */
typedef struct {
    const char* name;
    int (*test_func)(void);
} test_suite_t;

/* Test suite registry */
static const test_suite_t test_suites[] = {
    {"Memory Pool Tests", test_memory_main},
    {"SMOF Format Tests", test_smof_main},
    {"Error Handling Tests", test_error_main},
    {"Integration Tests", test_integration_main},
    {NULL, NULL}  /* Sentinel */
};

int main(void) {
    int total_failures = 0;
    int total_tests = 0;
    
    printf("STLD Test Suite\n");
    printf("===============\n\n");
    
    /* Run each test suite */
    for (const test_suite_t* suite = test_suites; suite->name != NULL; suite++) {
        int result;
        printf("Running %s...\n", suite->name);
        printf("----------------------------------------\n");
        
        result = suite->test_func();
        total_failures += result;
        total_tests++;
        
        if (result == 0) {
            printf("✓ %s PASSED\n\n", suite->name);
        } else {
            printf("✗ %s FAILED (%d failures)\n\n", suite->name, result);
        }
    }
    
    /* Print summary */
    printf("========================================\n");
    printf("Test Summary\n");
    printf("========================================\n");
    printf("Test Suites Run: %d\n", total_tests);
    printf("Test Suites Passed: %d\n", total_tests - (total_failures > 0 ? 1 : 0));
    printf("Test Suites Failed: %d\n", total_failures > 0 ? 1 : 0);
    printf("Total Failures: %d\n", total_failures);
    
    if (total_failures == 0) {
        printf("\n🎉 ALL TESTS PASSED! 🎉\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n❌ SOME TESTS FAILED ❌\n");
        return EXIT_FAILURE;
    }
}
