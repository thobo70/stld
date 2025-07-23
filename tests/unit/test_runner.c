/**
 * @file test_runner.c
 * @brief Main test runner for STLD/STAR unit test suite
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * Comprehensive test runner that executes all unit tests for the STLD linker
 * and STAR archiver components. Provides test reporting, failure tracking,
 * and integration with CI/CD systems.
 */

#include "unity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Test group function declarations */
extern int run_memory_tests(void);
extern int run_smof_tests(void);
extern int run_error_tests(void);
extern int run_symbol_table_tests(void);
extern int run_section_tests(void);
extern int run_relocation_tests(void);
extern int run_output_tests(void);
extern int run_archive_tests(void);
extern int run_compress_tests(void);
extern int run_index_tests(void);

/* Test group information structure */
typedef struct {
    const char* name;
    const char* description;
    int (*run_function)(void);
    bool enabled;
} test_group_t;

/* Test results tracking */
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
    int ignored_tests;
    double elapsed_time;
} test_results_t;

/* Global test results */
static test_results_t g_test_results = {0};

/**
 * @brief Print test runner banner
 */
static void print_banner(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    STLD/STAR Test Suite                     ║\n");
    printf("║                                                              ║\n");
    printf("║  Comprehensive unit tests for STLD linker and STAR archiver ║\n");
    printf("║  Version 1.0.0 - Built with Unity Testing Framework        ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

/**
 * @brief Print test group header
 */
static void print_group_header(const char* group_name, const char* description) {
    printf("┌─ Running %s Tests ─", group_name);
    for (int i = strlen(group_name) + 17; i < 60; i++) {
        printf("─");
    }
    printf("┐\n");
    printf("│ %s", description);
    for (int i = strlen(description) + 2; i < 60; i++) {
        printf(" ");
    }
    printf("│\n");
    printf("└");
    for (int i = 0; i < 60; i++) {
        printf("─");
    }
    printf("┘\n");
}

/**
 * @brief Print test group footer with results
 */
static void print_group_footer(const char* group_name, int result) {
    if (result == 0) {
        printf("✓ %s tests completed successfully\n\n", group_name);
    } else {
        printf("✗ %s tests failed with %d errors\n\n", group_name, result);
    }
}

/**
 * @brief Print final test summary
 */
static void print_summary(void) {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                        Test Summary                          ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Total Tests:    %6d                                    ║\n", g_test_results.total_tests);
    printf("║ Passed:         %6d                                    ║\n", g_test_results.passed_tests);
    printf("║ Failed:         %6d                                    ║\n", g_test_results.failed_tests);
    printf("║ Ignored:        %6d                                    ║\n", g_test_results.ignored_tests);
    printf("║ Elapsed Time:   %6.2f seconds                          ║\n", g_test_results.elapsed_time);
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    
    if (g_test_results.failed_tests == 0) {
        printf("║                    🎉 ALL TESTS PASSED! 🎉                  ║\n");
    } else {
        printf("║                   ❌ SOME TESTS FAILED ❌                   ║\n");
        printf("║                                                              ║\n");
        printf("║ Review the output above for details on failed tests.        ║\n");
    }
    
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

/**
 * @brief Parse command line arguments
 */
static bool parse_arguments(int argc, char* argv[], test_group_t* groups, size_t group_count) {
    if (argc == 1) {
        /* No arguments - run all tests */
        return true;
    }
    
    /* Check for help option */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("STLD/STAR Test Suite Usage:\n");
            printf("  %s [options] [test_groups...]\n\n", argv[0]);
            printf("Options:\n");
            printf("  -h, --help     Show this help message\n");
            printf("  --list         List available test groups\n");
            printf("  --verbose      Enable verbose output\n\n");
            printf("Test Groups:\n");
            for (size_t j = 0; j < group_count; j++) {
                printf("  %-15s %s\n", groups[j].name, groups[j].description);
            }
            printf("\nExamples:\n");
            printf("  %s                    # Run all tests\n", argv[0]);
            printf("  %s memory smof        # Run only memory and SMOF tests\n", argv[0]);
            printf("  %s --list             # List available test groups\n", argv[0]);
            return false;
        }
        
        if (strcmp(argv[i], "--list") == 0) {
            printf("Available test groups:\n");
            for (size_t j = 0; j < group_count; j++) {
                printf("  %-15s %s\n", groups[j].name, groups[j].description);
            }
            return false;
        }
        
        if (strcmp(argv[i], "--verbose") == 0) {
            Unity.TestFile = NULL;  /* Enable verbose output */
            continue;
        }
    }
    
    /* Disable all groups first */
    for (size_t i = 0; i < group_count; i++) {
        groups[i].enabled = false;
    }
    
    /* Enable specified groups */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') continue;  /* Skip options */
        
        bool found = false;
        for (size_t j = 0; j < group_count; j++) {
            if (strcmp(argv[i], groups[j].name) == 0) {
                groups[j].enabled = true;
                found = true;
                break;
            }
        }
        
        if (!found) {
            printf("Error: Unknown test group '%s'\n", argv[i]);
            printf("Use --list to see available test groups.\n");
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Update global test results from Unity
 */
static void update_test_results(void) {
    g_test_results.total_tests += Unity.NumberOfTests;
    g_test_results.passed_tests += Unity.NumberOfTests - Unity.TestFailures - Unity.TestIgnores;
    g_test_results.failed_tests += Unity.TestFailures;
    g_test_results.ignored_tests += Unity.TestIgnores;
}

/**
 * @brief Main test runner entry point
 */
int main(int argc, char* argv[]) {
    /* Test group definitions */
    test_group_t test_groups[] = {
        {
            .name = "memory",
            .description = "Memory pool management and allocation tests",
            .run_function = run_memory_tests,
            .enabled = true
        },
        {
            .name = "smof",
            .description = "SMOF format validation and parsing tests",
            .run_function = run_smof_tests,
            .enabled = true
        },
        {
            .name = "error",
            .description = "Error handling and context management tests",
            .run_function = run_error_tests,
            .enabled = true
        },
        {
            .name = "symbol",
            .description = "Symbol table management and resolution tests",
            .run_function = run_symbol_table_tests,
            .enabled = true
        },
        {
            .name = "section",
            .description = "Section management and layout tests",
            .run_function = run_section_tests,
            .enabled = true
        },
        {
            .name = "relocation",
            .description = "Relocation processing and address resolution tests",
            .run_function = run_relocation_tests,
            .enabled = true
        },
        {
            .name = "output",
            .description = "Output file generation and formatting tests",
            .run_function = run_output_tests,
            .enabled = true
        },
        {
            .name = "archive",
            .description = "Archive creation and file management tests",
            .run_function = run_archive_tests,
            .enabled = true
        },
        {
            .name = "compress",
            .description = "Compression engine and algorithm tests",
            .run_function = run_compress_tests,
            .enabled = true
        },
        {
            .name = "index",
            .description = "File indexing and search functionality tests",
            .run_function = run_index_tests,
            .enabled = true
        }
    };
    
    const size_t group_count = sizeof(test_groups) / sizeof(test_groups[0]);
    
    /* Parse command line arguments */
    if (!parse_arguments(argc, argv, test_groups, group_count)) {
        return 0;  /* Help or list requested */
    }
    
    /* Print banner */
    print_banner();
    
    /* Initialize timing */
    clock_t start_time = clock();
    
    /* Initialize Unity */
    UnityBegin("STLD/STAR Test Suite");
    
    /* Run enabled test groups */
    int overall_result = 0;
    int groups_run = 0;
    
    for (size_t i = 0; i < group_count; i++) {
        if (!test_groups[i].enabled) {
            continue;
        }
        
        groups_run++;
        
        /* Print group header */
        print_group_header(test_groups[i].name, test_groups[i].description);
        
        /* Reset Unity state for this group */
        Unity.NumberOfTests = 0;
        Unity.TestFailures = 0;
        Unity.TestIgnores = 0;
        
        /* Run the test group */
        int group_result = test_groups[i].run_function();
        
        /* Update global results */
        update_test_results();
        
        /* Print group footer */
        print_group_footer(test_groups[i].name, group_result);
        
        /* Update overall result */
        if (group_result != 0) {
            overall_result = group_result;
        }
    }
    
    /* Calculate elapsed time */
    clock_t end_time = clock();
    g_test_results.elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    /* Print summary */
    if (groups_run > 0) {
        print_summary();
    } else {
        printf("No test groups were executed.\n");
        printf("Use --help to see available options and test groups.\n");
    }
    
    /* Return appropriate exit code */
    return overall_result;
}
