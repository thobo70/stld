/* tests/test_integration.c */
#include "unity.h"
#include "smof.h"
#include "memory.h"
#include "error.h"
#include "stld.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @file test_integration.c
 * @brief Integration tests for STLD linker components
 * @details Tests interaction between SMOF parsing, memory management, and linking
 */

/* Test data */
static memory_pool_t test_pool;
static uint8_t test_buffer[4096];

void setUp(void) {
    memory_pool_init(&test_pool, test_buffer, sizeof(test_buffer));
    smof_error_clear();
}

void tearDown(void) {
    memory_pool_destroy(&test_pool);
    smof_error_clear();
}

void test_smof_with_memory_pool(void) {
    /* Create a simple SMOF header in memory */
    smof_header_t* header = (smof_header_t*)memory_pool_allocate(&test_pool, sizeof(smof_header_t));
    TEST_ASSERT_NOT_NULL(header);
    
    /* Initialize header */
    header->magic = SMOF_MAGIC;
    header->version = SMOF_CURRENT_VERSION;
    header->flags = 0;
    header->reserved = 0;
    header->header_size = sizeof(smof_header_t);
    header->section_count = 1;
    header->symbol_count = 0;
    header->relocation_count = 0;
    header->reserved2 = 0;
    header->reserved3 = 0;
    header->reserved4 = 0;
    header->reserved5 = 0;
    
    /* Validate the header */
    smof_error_t result = smof_validate_file((const uint8_t*)header, sizeof(smof_header_t));
    TEST_ASSERT_EQUAL_INT(SMOF_SUCCESS, result);
    
    /* Parse header back */
    smof_header_t parsed_header;
    result = smof_parse_header((const uint8_t*)header, sizeof(smof_header_t), &parsed_header);
    TEST_ASSERT_EQUAL_INT(SMOF_SUCCESS, result);
    TEST_ASSERT_EQUAL_UINT32(SMOF_MAGIC, parsed_header.magic);
    TEST_ASSERT_EQUAL_UINT16(SMOF_CURRENT_VERSION, parsed_header.version);
}

void test_error_handling_with_memory_allocation(void) {
    /* Try to allocate more than available */
    void* ptr = memory_pool_allocate(&test_pool, sizeof(test_buffer) + 1000);
    TEST_ASSERT_NULL(ptr);
    
    /* Set an error to simulate allocation failure */
    smof_error_set(SMOF_ERROR_OUT_OF_MEMORY, "Failed to allocate object file buffer");
    TEST_ASSERT_TRUE(smof_error_has_error());
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_OUT_OF_MEMORY, smof_error_get_last());
    
    /* Error should persist across other operations */
    void* small_ptr = memory_pool_allocate(&test_pool, 64);
    TEST_ASSERT_NOT_NULL(small_ptr);  /* This should work */
    TEST_ASSERT_TRUE(smof_error_has_error());  /* Error should still be set */
}

void test_linker_context_initialization(void) {
    stld_context_t* ctx = (stld_context_t*)memory_pool_allocate(&test_pool, sizeof(stld_context_t));
    TEST_ASSERT_NOT_NULL(ctx);
    
    /* Initialize linker context */
    smof_error_t result = stld_init(ctx);
    TEST_ASSERT_EQUAL_INT(SMOF_SUCCESS, result);
    
    /* Verify initialization */
    TEST_ASSERT_NOT_NULL(ctx->symbol_table);
    TEST_ASSERT_NOT_NULL(ctx->sections);
    TEST_ASSERT_EQUAL_UINT(0, ctx->num_sections);
    TEST_ASSERT_EQUAL_UINT(0, ctx->num_objects);
    TEST_ASSERT_NULL(ctx->output_format);
    
    /* Cleanup */
    stld_cleanup(ctx);
}

void test_multiple_object_files_simulation(void) {
    /* Simulate processing multiple object files */
    const int num_files = 3;
    smof_header_t* headers[num_files];
    
    for (int i = 0; i < num_files; i++) {
        headers[i] = (smof_header_t*)memory_pool_allocate(&test_pool, sizeof(smof_header_t));
        TEST_ASSERT_NOT_NULL(headers[i]);
        
        /* Initialize each header */
        headers[i]->magic = SMOF_MAGIC;
        headers[i]->version = SMOF_CURRENT_VERSION;
        headers[i]->flags = 0;
        headers[i]->reserved = 0;
        headers[i]->header_size = sizeof(smof_header_t);
        headers[i]->section_count = i + 1;  /* Different section counts */
        headers[i]->symbol_count = (i + 1) * 5;  /* Different symbol counts */
        headers[i]->relocation_count = i * 2;
        headers[i]->reserved2 = 0;
        headers[i]->reserved3 = 0;
        headers[i]->reserved4 = 0;
        headers[i]->reserved5 = 0;
        
        /* Validate each header */
        smof_error_t result = smof_validate_file((const uint8_t*)headers[i], sizeof(smof_header_t));
        TEST_ASSERT_EQUAL_INT(SMOF_SUCCESS, result);
    }
    
    /* Verify we can access all headers */
    for (int i = 0; i < num_files; i++) {
        TEST_ASSERT_EQUAL_UINT32(SMOF_MAGIC, headers[i]->magic);
        TEST_ASSERT_EQUAL_UINT32(i + 1, headers[i]->section_count);
        TEST_ASSERT_EQUAL_UINT32((i + 1) * 5, headers[i]->symbol_count);
    }
}

void test_memory_pool_with_sections(void) {
    /* Allocate space for sections */
    const size_t num_sections = 5;
    smof_section_header_t* sections = (smof_section_header_t*)
        memory_pool_allocate(&test_pool, num_sections * sizeof(smof_section_header_t));
    TEST_ASSERT_NOT_NULL(sections);
    
    /* Initialize sections */
    for (size_t i = 0; i < num_sections; i++) {
        sections[i].name_offset = i * 16;
        sections[i].type = (uint8_t)(i % 7);  /* Cycle through section types */
        sections[i].flags = 0;
        sections[i].reserved = 0;
        sections[i].size = (i + 1) * 1024;
        sections[i].data_offset = 0;
        sections[i].alignment = 4;
        sections[i].reserved2 = 0;
    }
    
    /* Verify sections are properly initialized */
    for (size_t i = 0; i < num_sections; i++) {
        TEST_ASSERT_EQUAL_UINT32(i * 16, sections[i].name_offset);
        TEST_ASSERT_EQUAL_UINT8(i % 7, sections[i].type);
        TEST_ASSERT_EQUAL_UINT32((i + 1) * 1024, sections[i].size);
        TEST_ASSERT_EQUAL_UINT32(4, sections[i].alignment);
    }
    
    /* Check memory pool usage */
    memory_pool_stats_t stats;
    memory_pool_get_stats(&test_pool, &stats);
    TEST_ASSERT_TRUE(stats.used_size >= num_sections * sizeof(smof_section_header_t));
}

void test_error_recovery_scenario(void) {
    /* Simulate an error during processing */
    smof_error_set(SMOF_ERROR_INVALID_MAGIC, "Corrupted object file");
    TEST_ASSERT_TRUE(smof_error_has_error());
    
    /* Try to continue processing (should detect error) */
    if (smof_error_has_error()) {
        /* Log error and attempt recovery */
        const char* error_msg = smof_error_get_message();
        TEST_ASSERT_NOT_NULL(error_msg);
        TEST_ASSERT_TRUE(strlen(error_msg) > 0);
        
        /* Clear error and retry */
        smof_error_clear();
        TEST_ASSERT_FALSE(smof_error_has_error());
        
        /* Now we can proceed */
        void* recovery_ptr = memory_pool_allocate(&test_pool, 128);
        TEST_ASSERT_NOT_NULL(recovery_ptr);
    }
}

void test_full_workflow_simulation(void) {
    /* Simulate a complete linking workflow */
    
    /* 1. Initialize linker context */
    stld_context_t* ctx = (stld_context_t*)memory_pool_allocate(&test_pool, sizeof(stld_context_t));
    TEST_ASSERT_NOT_NULL(ctx);
    
    smof_error_t result = stld_init(ctx);
    TEST_ASSERT_EQUAL_INT(SMOF_SUCCESS, result);
    
    /* 2. Create and validate object file header */
    smof_header_t* header = (smof_header_t*)memory_pool_allocate(&test_pool, sizeof(smof_header_t));
    TEST_ASSERT_NOT_NULL(header);
    
    header->magic = SMOF_MAGIC;
    header->version = SMOF_CURRENT_VERSION;
    header->flags = 0;
    header->reserved = 0;
    header->header_size = sizeof(smof_header_t);
    header->section_count = 3;
    header->symbol_count = 10;
    header->relocation_count = 5;
    header->reserved2 = 0;
    header->reserved3 = 0;
    header->reserved4 = 0;
    header->reserved5 = 0;
    
    result = smof_validate_file((const uint8_t*)header, sizeof(smof_header_t));
    TEST_ASSERT_EQUAL_INT(SMOF_SUCCESS, result);
    
    /* 3. Allocate sections */
    smof_section_header_t* sections = (smof_section_header_t*)
        memory_pool_allocate(&test_pool, header->section_count * sizeof(smof_section_header_t));
    TEST_ASSERT_NOT_NULL(sections);
    
    /* 4. Initialize sections */
    sections[0].type = SMOF_SECTION_TEXT;
    sections[0].size = 1024;
    sections[1].type = SMOF_SECTION_DATA;
    sections[1].size = 512;
    sections[2].type = SMOF_SECTION_BSS;
    sections[2].size = 256;
    
    /* 5. Check memory usage */
    memory_pool_stats_t stats;
    memory_pool_get_stats(&test_pool, &stats);
    TEST_ASSERT_TRUE(stats.used_size > 0);
    TEST_ASSERT_TRUE(stats.free_size > 0);
    
    /* 6. Cleanup */
    stld_cleanup(ctx);
    
    /* 7. Reset memory pool for next use */
    memory_pool_reset(&test_pool);
    memory_pool_get_stats(&test_pool, &stats);
    TEST_ASSERT_EQUAL_UINT(0, stats.used_size);
}

int test_integration_main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_smof_with_memory_pool);
    RUN_TEST(test_error_handling_with_memory_allocation);
    RUN_TEST(test_linker_context_initialization);
    RUN_TEST(test_multiple_object_files_simulation);
    RUN_TEST(test_memory_pool_with_sections);
    RUN_TEST(test_error_recovery_scenario);
    RUN_TEST(test_full_workflow_simulation);
    
    return UNITY_END();
}

int main(void) {
    return test_integration_main();
}
