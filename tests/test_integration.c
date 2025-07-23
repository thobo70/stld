/* tests/test_integration.c */
#include "unity.h"
#include "smof.h"
#include "memory.h"
#include "error.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @file test_integration.c
 * @brief Integration tests for STLD system components
 * @details Tests interaction between SMOF parsing, memory management, and error handling
 */

/* Function prototypes for C90 compliance */
void test_smof_with_memory_pool(void);
void test_error_handling_with_memory_allocation(void);
void test_memory_pool_with_sections(void);
void test_error_recovery_scenario(void);
void test_full_workflow_simulation(void);
int test_integration_main(void);

/* Test data */
static memory_pool_t* test_pool;
static uint8_t test_buffer[4096];

void setUp(void) {
    /* Create memory pool using current API */
    test_pool = memory_pool_create(sizeof(test_buffer));
    TEST_ASSERT_NOT_NULL(test_pool);
}

void tearDown(void) {
    /* Clean up memory pool */
    if (test_pool != NULL) {
        memory_pool_destroy(test_pool);
        test_pool = NULL;
    }
}

void test_smof_with_memory_pool(void) {
    /* Create a simple SMOF header in memory using current API */
    smof_header_t* header = (smof_header_t*)memory_pool_alloc(test_pool, sizeof(smof_header_t));
    TEST_ASSERT_NOT_NULL(header);
    
    /* Initialize header with current structure fields */
    header->magic = SMOF_MAGIC;
    header->version = SMOF_VERSION;
    header->flags = SMOF_FLAG_LITTLE_ENDIAN;
    header->entry_point = 0x1000;
    header->section_count = 1;
    header->symbol_count = 0;
    header->section_table_offset = 64;   /* Must be > sizeof(smof_header_t) */
    header->symbol_table_offset = 128;   /* Must be > sizeof(smof_header_t) */
    header->string_table_offset = 256;   /* Must be > sizeof(smof_header_t) */
    header->checksum = 0;
    
    /* Validate the header using current API */
    TEST_ASSERT_TRUE(smof_validate_header(header));
    TEST_ASSERT_TRUE(smof_header_is_valid(header));
    
    /* Test header parsing functions */
    TEST_ASSERT_EQUAL_UINT(SMOF_MAGIC, header->magic);
    TEST_ASSERT_EQUAL_UINT(SMOF_VERSION, header->version);
    TEST_ASSERT_EQUAL_UINT(SMOF_FLAG_LITTLE_ENDIAN, header->flags);
}

void test_error_handling_with_memory_allocation(void) {
    void* ptr;
    void* small_ptr;
    
    /* Try to allocate more memory than available */
    ptr = memory_pool_alloc(test_pool, memory_pool_get_size(test_pool) + 1000);
    TEST_ASSERT_NULL(ptr);
    
    /* Test error reporting using current API */
    error_report(ERROR_OUT_OF_MEMORY, ERROR_SEVERITY_ERROR,
                __FILE__, __LINE__, __func__, "Failed to allocate object file buffer");
    
    /* Test smaller allocation should succeed */
    small_ptr = memory_pool_alloc(test_pool, 64);
    TEST_ASSERT_NOT_NULL(small_ptr);
}

void test_memory_pool_with_sections(void) {
    /* Allocate space for multiple sections using current API */
    const size_t num_sections = 3;
    smof_section_header_t* sections;
    size_t i;
    memory_pool_stats_t stats;
    
    sections = (smof_section_header_t*)
        memory_pool_alloc(test_pool, num_sections * sizeof(smof_section_header_t));
    
    TEST_ASSERT_NOT_NULL(sections);
    
    /* Initialize sections with current structure fields */
    for (i = 0; i < num_sections; i++) {
        sections[i].name_offset = (uint32_t)(i * 16);
        sections[i].type = (uint32_t)(i % 7);  /* Cycle through section types */
        sections[i].flags = 0;
        sections[i].addr = 0x1000 + (uint32_t)(i * 0x1000);
        sections[i].size = (uint32_t)((i + 1) * 1024);
        sections[i].offset = 0;
        sections[i].link = 0;
        sections[i].info = 0;
        sections[i].alignment = 4;
        sections[i].entry_size = 0;
    }
    
    /* Verify section data */
    for (i = 0; i < num_sections; i++) {
        TEST_ASSERT_EQUAL_UINT((uint32_t)(i % 7), sections[i].type);
        TEST_ASSERT_TRUE(sections[i].size > 0);
    }
    
    /* Check memory pool statistics */
    memory_pool_get_stats(test_pool, &stats);
    TEST_ASSERT_TRUE(stats.used_size > 0);
    TEST_ASSERT_TRUE(stats.allocations > 0);
}

void test_error_recovery_scenario(void) {
    const char* error_string;
    void* recovery_ptr;
    
    /* Simulate error scenario using current error API */
    error_report(ERROR_INVALID_MAGIC, ERROR_SEVERITY_ERROR,
                __FILE__, __LINE__, __func__, "Corrupted object file");
    
    /* Test error utility functions */
    TEST_ASSERT_TRUE(error_is_failure(ERROR_INVALID_MAGIC));
    TEST_ASSERT_FALSE(error_is_success(ERROR_INVALID_MAGIC));
    
    /* Test error string conversion */
    error_string = error_get_string(ERROR_INVALID_MAGIC);
    TEST_ASSERT_NOT_NULL(error_string);
    TEST_ASSERT_EQUAL_STRING("Invalid magic number", error_string);
    
    /* Reset pool and continue - test recovery */
    memory_pool_reset(test_pool);
    
    /* Should be able to allocate again after reset */
    recovery_ptr = memory_pool_alloc(test_pool, 128);
    TEST_ASSERT_NOT_NULL(recovery_ptr);
}

void test_full_workflow_simulation(void) {
    memory_pool_stats_t stats;
    smof_header_t* header;
    smof_section_header_t* sections;
    
    /* Simulate a complete workflow: allocation, processing, cleanup */
    
    /* Step 1: Allocate SMOF header */
    header = (smof_header_t*)memory_pool_alloc(test_pool, sizeof(smof_header_t));
    TEST_ASSERT_NOT_NULL(header);
    
    /* Step 2: Initialize with valid data */
    header->magic = SMOF_MAGIC;
    header->version = SMOF_VERSION;
    header->flags = SMOF_FLAG_LITTLE_ENDIAN;
    header->entry_point = 0x1000;
    header->section_count = 3;
    header->symbol_count = 10;
    header->section_table_offset = 32;
    header->symbol_table_offset = 152;
    header->string_table_offset = 312;
    header->checksum = 0;
    
    /* Step 3: Validate header */
    TEST_ASSERT_TRUE(smof_validate_header(header));
    
    /* Step 4: Allocate sections */
    sections = (smof_section_header_t*)
        memory_pool_alloc(test_pool, header->section_count * sizeof(smof_section_header_t));
    TEST_ASSERT_NOT_NULL(sections);
    
    /* Step 5: Initialize sections */
    sections[0].type = SMOF_SECTION_PROGBITS;
    sections[0].flags = 0x6;  /* Read+Execute */
    
    sections[1].type = SMOF_SECTION_PROGBITS;
    sections[1].flags = 0x3;  /* Read+Write */
    
    sections[2].type = SMOF_SECTION_STRTAB;
    sections[2].flags = 0x0;  /* No flags */
    
    /* Step 6: Check memory usage */
    memory_pool_get_stats(test_pool, &stats);
    TEST_ASSERT_TRUE(stats.used_size > 0);
    TEST_ASSERT_TRUE(memory_pool_get_available(test_pool) < memory_pool_get_size(test_pool));
    
    /* Step 7: Reset and verify cleanup */
    memory_pool_reset(test_pool);
    memory_pool_get_stats(test_pool, &stats);
    TEST_ASSERT_EQUAL_UINT(0, (uint32_t)stats.used_size);
}

int test_integration_main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_smof_with_memory_pool);
    RUN_TEST(test_error_handling_with_memory_allocation);
    RUN_TEST(test_memory_pool_with_sections);
    RUN_TEST(test_error_recovery_scenario);
    RUN_TEST(test_full_workflow_simulation);
    
    return UNITY_END();
}

int main(void) {
    return test_integration_main();
}
