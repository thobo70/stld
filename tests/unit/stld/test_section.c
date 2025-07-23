/**
 * @file test_section.c
 * @brief Unit tests for STLD section management
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * Comprehensive unit tests for section handling including section creation,
 * memory layout, alignment, merging, and address assignment. Tests cover
 * text, data, BSS, and custom sections.
 */

#include "unity.h"
#include "section.h"
#include "memory.h"
#include "error.h"
#include <string.h>

/* Test fixture data */
static section_manager_t* test_manager;
static error_context_t* test_error_ctx;
static memory_pool_t* test_memory_pool;

/**
 * @brief Setup function called before each test
 */
void setUp(void) {
    test_error_ctx = error_context_create();
    TEST_ASSERT_NOT_NULL(test_error_ctx);
    
    test_memory_pool = memory_pool_create(8192);
    TEST_ASSERT_NOT_NULL(test_memory_pool);
    
    test_manager = section_manager_create(test_memory_pool, test_error_ctx);
    TEST_ASSERT_NOT_NULL(test_manager);
}

/**
 * @brief Teardown function called after each test
 */
void tearDown(void) {
    if (test_manager != NULL) {
        section_manager_destroy(test_manager);
        test_manager = NULL;
    }
    
    if (test_memory_pool != NULL) {
        memory_pool_destroy(test_memory_pool);
        test_memory_pool = NULL;
    }
    
    if (test_error_ctx != NULL) {
        error_context_destroy(test_error_ctx);
        test_error_ctx = NULL;
    }
}

/**
 * @brief Test section manager creation and destruction
 */
void test_section_manager_lifecycle(void) {
    section_manager_t* manager = section_manager_create(test_memory_pool, test_error_ctx);
    TEST_ASSERT_NOT_NULL(manager);
    
    /* Initial state */
    TEST_ASSERT_EQUAL_size_t(0, section_manager_get_count(manager));
    TEST_ASSERT_TRUE(section_manager_is_empty(manager));
    
    section_manager_destroy(manager);
    
    /* Test with NULL parameters */
    manager = section_manager_create(NULL, test_error_ctx);
    TEST_ASSERT_NULL(manager);
    
    manager = section_manager_create(test_memory_pool, NULL);
    TEST_ASSERT_NULL(manager);
    
    /* Test destruction with NULL */
    section_manager_destroy(NULL); /* Should not crash */
}

/**
 * @brief Test basic section creation
 */
void test_section_creation_basic(void) {
    /* Create text section */
    section_id_t text_id = section_manager_create_section(test_manager, ".text", 
                                                         SECTION_TYPE_TEXT, 
                                                         SECTION_FLAG_EXECUTABLE | SECTION_FLAG_READABLE);
    TEST_ASSERT_NOT_EQUAL(SECTION_ID_INVALID, text_id);
    
    /* Create data section */
    section_id_t data_id = section_manager_create_section(test_manager, ".data",
                                                         SECTION_TYPE_DATA,
                                                         SECTION_FLAG_READABLE | SECTION_FLAG_WRITABLE);
    TEST_ASSERT_NOT_EQUAL(SECTION_ID_INVALID, data_id);
    
    /* Create BSS section */
    section_id_t bss_id = section_manager_create_section(test_manager, ".bss",
                                                        SECTION_TYPE_BSS,
                                                        SECTION_FLAG_READABLE | SECTION_FLAG_WRITABLE);
    TEST_ASSERT_NOT_EQUAL(SECTION_ID_INVALID, bss_id);
    
    /* Verify manager state */
    TEST_ASSERT_EQUAL_size_t(3, section_manager_get_count(test_manager));
    TEST_ASSERT_FALSE(section_manager_is_empty(test_manager));
    
    /* Verify sections are different */
    TEST_ASSERT_NOT_EQUAL(text_id, data_id);
    TEST_ASSERT_NOT_EQUAL(data_id, bss_id);
    TEST_ASSERT_NOT_EQUAL(text_id, bss_id);
    
    /* Verify no errors occurred */
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Test section retrieval and properties
 */
void test_section_retrieval(void) {
    /* Create test section */
    section_id_t section_id = section_manager_create_section(test_manager, ".custom",
                                                            SECTION_TYPE_PROGBITS,
                                                            SECTION_FLAG_READABLE);
    TEST_ASSERT_NOT_EQUAL(SECTION_ID_INVALID, section_id);
    
    /* Retrieve section */
    const section_t* section = section_manager_get_section(test_manager, section_id);
    TEST_ASSERT_NOT_NULL(section);
    
    /* Verify properties */
    TEST_ASSERT_EQUAL_STRING(".custom", section_get_name(section));
    TEST_ASSERT_EQUAL_INT(SECTION_TYPE_PROGBITS, section_get_type(section));
    TEST_ASSERT_EQUAL_INT(SECTION_FLAG_READABLE, section_get_flags(section));
    TEST_ASSERT_EQUAL_HEX32(0, section_get_address(section)); /* Not assigned yet */
    TEST_ASSERT_EQUAL_size_t(0, section_get_size(section)); /* No data yet */
    TEST_ASSERT_EQUAL_size_t(1, section_get_alignment(section)); /* Default alignment */
    
    /* Test invalid section ID */
    const section_t* invalid = section_manager_get_section(test_manager, SECTION_ID_INVALID);
    TEST_ASSERT_NULL(invalid);
    
    section_id_t bad_id = (section_id_t)(section_manager_get_count(test_manager) + 100);
    const section_t* bad_section = section_manager_get_section(test_manager, bad_id);
    TEST_ASSERT_NULL(bad_section);
}

/**
 * @brief Test section lookup by name
 */
void test_section_lookup_by_name(void) {
    /* Create test sections */
    section_id_t text_id = section_manager_create_section(test_manager, ".text", SECTION_TYPE_TEXT, SECTION_FLAG_EXECUTABLE);
    section_id_t data_id = section_manager_create_section(test_manager, ".data", SECTION_TYPE_DATA, SECTION_FLAG_WRITABLE);
    section_id_t rodata_id = section_manager_create_section(test_manager, ".rodata", SECTION_TYPE_PROGBITS, SECTION_FLAG_READABLE);
    
    /* Test successful lookups */
    section_id_t found_text = section_manager_find_section(test_manager, ".text");
    section_id_t found_data = section_manager_find_section(test_manager, ".data");
    section_id_t found_rodata = section_manager_find_section(test_manager, ".rodata");
    
    TEST_ASSERT_EQUAL(text_id, found_text);
    TEST_ASSERT_EQUAL(data_id, found_data);
    TEST_ASSERT_EQUAL(rodata_id, found_rodata);
    
    /* Test failed lookup */
    section_id_t not_found = section_manager_find_section(test_manager, ".nonexistent");
    TEST_ASSERT_EQUAL(SECTION_ID_INVALID, not_found);
    
    /* Test NULL name */
    section_id_t null_lookup = section_manager_find_section(test_manager, NULL);
    TEST_ASSERT_EQUAL(SECTION_ID_INVALID, null_lookup);
}

/**
 * @brief Test section data management
 */
void test_section_data_management(void) {
    /* Create section */
    section_id_t section_id = section_manager_create_section(test_manager, ".data", SECTION_TYPE_DATA, SECTION_FLAG_WRITABLE);
    TEST_ASSERT_NOT_EQUAL(SECTION_ID_INVALID, section_id);
    
    /* Add data to section */
    uint8_t test_data[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    bool result = section_manager_add_data(test_manager, section_id, test_data, sizeof(test_data));
    TEST_ASSERT_TRUE(result);
    
    /* Verify section size updated */
    const section_t* section = section_manager_get_section(test_manager, section_id);
    TEST_ASSERT_EQUAL_size_t(sizeof(test_data), section_get_size(section));
    
    /* Retrieve and verify data */
    const uint8_t* retrieved_data = section_get_data(section);
    TEST_ASSERT_NOT_NULL(retrieved_data);
    TEST_ASSERT_EQUAL_MEMORY(test_data, retrieved_data, sizeof(test_data));
    
    /* Add more data */
    uint8_t more_data[] = {0xFF, 0xEE, 0xDD, 0xCC};
    result = section_manager_add_data(test_manager, section_id, more_data, sizeof(more_data));
    TEST_ASSERT_TRUE(result);
    
    /* Verify total size */
    TEST_ASSERT_EQUAL_size_t(sizeof(test_data) + sizeof(more_data), section_get_size(section));
    
    /* Verify concatenated data */
    retrieved_data = section_get_data(section);
    TEST_ASSERT_EQUAL_MEMORY(test_data, retrieved_data, sizeof(test_data));
    TEST_ASSERT_EQUAL_MEMORY(more_data, retrieved_data + sizeof(test_data), sizeof(more_data));
}

/**
 * @brief Test section alignment handling
 */
void test_section_alignment(void) {
    /* Create section with specific alignment */
    section_id_t section_id = section_manager_create_section(test_manager, ".aligned", SECTION_TYPE_PROGBITS, SECTION_FLAG_READABLE);
    TEST_ASSERT_NOT_EQUAL(SECTION_ID_INVALID, section_id);
    
    /* Set alignment */
    bool result = section_manager_set_alignment(test_manager, section_id, 16);
    TEST_ASSERT_TRUE(result);
    
    const section_t* section = section_manager_get_section(test_manager, section_id);
    TEST_ASSERT_EQUAL_size_t(16, section_get_alignment(section));
    
    /* Test invalid alignments */
    result = section_manager_set_alignment(test_manager, section_id, 3); /* Not power of 2 */
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
    
    error_context_clear(test_error_ctx);
    
    result = section_manager_set_alignment(test_manager, section_id, 0); /* Zero alignment */
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
    
    /* Verify alignment didn't change */
    TEST_ASSERT_EQUAL_size_t(16, section_get_alignment(section));
}

/**
 * @brief Test section address assignment
 */
void test_section_address_assignment(void) {
    /* Create sections */
    section_id_t text_id = section_manager_create_section(test_manager, ".text", SECTION_TYPE_TEXT, SECTION_FLAG_EXECUTABLE);
    section_id_t data_id = section_manager_create_section(test_manager, ".data", SECTION_TYPE_DATA, SECTION_FLAG_WRITABLE);
    
    /* Add some data to establish sizes */
    uint8_t text_data[256] = {0};
    uint8_t data_data[128] = {0};
    
    section_manager_add_data(test_manager, text_id, text_data, sizeof(text_data));
    section_manager_add_data(test_manager, data_id, data_data, sizeof(data_data));
    
    /* Assign addresses */
    bool result = section_manager_assign_address(test_manager, text_id, 0x1000);
    TEST_ASSERT_TRUE(result);
    
    result = section_manager_assign_address(test_manager, data_id, 0x2000);
    TEST_ASSERT_TRUE(result);
    
    /* Verify addresses */
    const section_t* text_section = section_manager_get_section(test_manager, text_id);
    const section_t* data_section = section_manager_get_section(test_manager, data_id);
    
    TEST_ASSERT_EQUAL_HEX32(0x1000, section_get_address(text_section));
    TEST_ASSERT_EQUAL_HEX32(0x2000, section_get_address(data_section));
    
    /* Test address ranges */
    TEST_ASSERT_EQUAL_HEX32(0x1000, section_get_start_address(text_section));
    TEST_ASSERT_EQUAL_HEX32(0x1000 + sizeof(text_data), section_get_end_address(text_section));
    
    TEST_ASSERT_EQUAL_HEX32(0x2000, section_get_start_address(data_section));
    TEST_ASSERT_EQUAL_HEX32(0x2000 + sizeof(data_data), section_get_end_address(data_section));
}

/**
 * @brief Test section merging
 */
void test_section_merging(void) {
    /* Create multiple sections with same name */
    section_id_t section1_id = section_manager_create_section(test_manager, ".data", SECTION_TYPE_DATA, SECTION_FLAG_WRITABLE);
    section_id_t section2_id = section_manager_create_section(test_manager, ".data", SECTION_TYPE_DATA, SECTION_FLAG_WRITABLE);
    
    TEST_ASSERT_NOT_EQUAL(SECTION_ID_INVALID, section1_id);
    TEST_ASSERT_NOT_EQUAL(SECTION_ID_INVALID, section2_id);
    
    /* Add data to both sections */
    uint8_t data1[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t data2[] = {0x55, 0x66, 0x77, 0x88};
    
    section_manager_add_data(test_manager, section1_id, data1, sizeof(data1));
    section_manager_add_data(test_manager, section2_id, data2, sizeof(data2));
    
    /* Merge sections */
    section_id_t merged_id = section_manager_merge_sections(test_manager, section1_id, section2_id);
    TEST_ASSERT_NOT_EQUAL(SECTION_ID_INVALID, merged_id);
    
    /* Verify merged section */
    const section_t* merged_section = section_manager_get_section(test_manager, merged_id);
    TEST_ASSERT_NOT_NULL(merged_section);
    TEST_ASSERT_EQUAL_size_t(sizeof(data1) + sizeof(data2), section_get_size(merged_section));
    
    /* Verify merged data */
    const uint8_t* merged_data = section_get_data(merged_section);
    TEST_ASSERT_EQUAL_MEMORY(data1, merged_data, sizeof(data1));
    TEST_ASSERT_EQUAL_MEMORY(data2, merged_data + sizeof(data1), sizeof(data2));
    
    /* Original sections should be invalidated */
    const section_t* invalid_section1 = section_manager_get_section(test_manager, section1_id);
    const section_t* invalid_section2 = section_manager_get_section(test_manager, section2_id);
    TEST_ASSERT_NULL(invalid_section1);
    TEST_ASSERT_NULL(invalid_section2);
}

/**
 * @brief Test section layout calculation
 */
void test_section_layout(void) {
    /* Create sections with different alignments */
    section_id_t text_id = section_manager_create_section(test_manager, ".text", SECTION_TYPE_TEXT, SECTION_FLAG_EXECUTABLE);
    section_id_t data_id = section_manager_create_section(test_manager, ".data", SECTION_TYPE_DATA, SECTION_FLAG_WRITABLE);
    section_id_t bss_id = section_manager_create_section(test_manager, ".bss", SECTION_TYPE_BSS, SECTION_FLAG_WRITABLE);
    
    /* Set alignments */
    section_manager_set_alignment(test_manager, text_id, 4);
    section_manager_set_alignment(test_manager, data_id, 8);
    section_manager_set_alignment(test_manager, bss_id, 16);
    
    /* Add data */
    uint8_t text_data[100] = {0};
    uint8_t data_data[50] = {0};
    
    section_manager_add_data(test_manager, text_id, text_data, sizeof(text_data));
    section_manager_add_data(test_manager, data_id, data_data, sizeof(data_data));
    section_manager_set_size(test_manager, bss_id, 200); /* BSS has no data but has size */
    
    /* Calculate layout starting at base address */
    uint32_t base_address = 0x10000;
    bool result = section_manager_calculate_layout(test_manager, base_address);
    TEST_ASSERT_TRUE(result);
    
    /* Verify addresses are properly aligned */
    const section_t* text_section = section_manager_get_section(test_manager, text_id);
    const section_t* data_section = section_manager_get_section(test_manager, data_id);
    const section_t* bss_section = section_manager_get_section(test_manager, bss_id);
    
    uint32_t text_addr = section_get_address(text_section);
    uint32_t data_addr = section_get_address(data_section);
    uint32_t bss_addr = section_get_address(bss_section);
    
    /* Text should start at base (aligned to 4) */
    TEST_ASSERT_EQUAL_HEX32(base_address, text_addr);
    TEST_ASSERT_EQUAL_INT(0, text_addr % 4);
    
    /* Data should follow text (aligned to 8) */
    uint32_t expected_data_addr = (text_addr + sizeof(text_data) + 7) & ~7;
    TEST_ASSERT_EQUAL_HEX32(expected_data_addr, data_addr);
    TEST_ASSERT_EQUAL_INT(0, data_addr % 8);
    
    /* BSS should follow data (aligned to 16) */
    uint32_t expected_bss_addr = (data_addr + sizeof(data_data) + 15) & ~15;
    TEST_ASSERT_EQUAL_HEX32(expected_bss_addr, bss_addr);
    TEST_ASSERT_EQUAL_INT(0, bss_addr % 16);
}

/**
 * @brief Test section iteration
 */
void test_section_iteration(void) {
    /* Create multiple sections */
    const char* section_names[] = {".text", ".data", ".rodata", ".bss"};
    const size_t count = sizeof(section_names) / sizeof(section_names[0]);
    
    section_id_t section_ids[count];
    
    for (size_t i = 0; i < count; i++) {
        section_ids[i] = section_manager_create_section(test_manager, section_names[i], 
                                                       SECTION_TYPE_PROGBITS, SECTION_FLAG_READABLE);
        TEST_ASSERT_NOT_EQUAL(SECTION_ID_INVALID, section_ids[i]);
    }
    
    /* Iterate through sections */
    size_t iteration_count = 0;
    bool found_sections[count];
    memset(found_sections, 0, sizeof(found_sections));
    
    section_iterator_t iter = section_manager_begin(test_manager);
    while (section_iterator_is_valid(&iter)) {
        const section_t* section = section_iterator_get(&iter);
        TEST_ASSERT_NOT_NULL(section);
        
        const char* name = section_get_name(section);
        
        /* Find which section this is */
        for (size_t i = 0; i < count; i++) {
            if (strcmp(name, section_names[i]) == 0) {
                found_sections[i] = true;
                break;
            }
        }
        
        iteration_count++;
        section_iterator_next(&iter);
    }
    
    /* Verify all sections were found */
    TEST_ASSERT_EQUAL_size_t(count, iteration_count);
    for (size_t i = 0; i < count; i++) {
        TEST_ASSERT_TRUE_MESSAGE(found_sections[i], "Section not found during iteration");
    }
}

/**
 * @brief Test section filtering by type
 */
void test_section_filtering(void) {
    /* Create sections of different types */
    section_id_t text_id = section_manager_create_section(test_manager, ".text", SECTION_TYPE_TEXT, SECTION_FLAG_EXECUTABLE);
    section_id_t data_id = section_manager_create_section(test_manager, ".data", SECTION_TYPE_DATA, SECTION_FLAG_WRITABLE);
    section_id_t bss_id = section_manager_create_section(test_manager, ".bss", SECTION_TYPE_BSS, SECTION_FLAG_WRITABLE);
    section_id_t note_id = section_manager_create_section(test_manager, ".note", SECTION_TYPE_NOTE, 0);
    
    /* Filter by type */
    section_list_t* text_sections = section_manager_filter_by_type(test_manager, SECTION_TYPE_TEXT);
    section_list_t* data_sections = section_manager_filter_by_type(test_manager, SECTION_TYPE_DATA);
    section_list_t* bss_sections = section_manager_filter_by_type(test_manager, SECTION_TYPE_BSS);
    section_list_t* note_sections = section_manager_filter_by_type(test_manager, SECTION_TYPE_NOTE);
    
    TEST_ASSERT_NOT_NULL(text_sections);
    TEST_ASSERT_NOT_NULL(data_sections);
    TEST_ASSERT_NOT_NULL(bss_sections);
    TEST_ASSERT_NOT_NULL(note_sections);
    
    TEST_ASSERT_EQUAL_size_t(1, section_list_size(text_sections));
    TEST_ASSERT_EQUAL_size_t(1, section_list_size(data_sections));
    TEST_ASSERT_EQUAL_size_t(1, section_list_size(bss_sections));
    TEST_ASSERT_EQUAL_size_t(1, section_list_size(note_sections));
    
    /* Verify correct sections were filtered */
    const section_t* text_sec = section_list_get(text_sections, 0);
    const section_t* data_sec = section_list_get(data_sections, 0);
    const section_t* bss_sec = section_list_get(bss_sections, 0);
    const section_t* note_sec = section_list_get(note_sections, 0);
    
    TEST_ASSERT_EQUAL_STRING(".text", section_get_name(text_sec));
    TEST_ASSERT_EQUAL_STRING(".data", section_get_name(data_sec));
    TEST_ASSERT_EQUAL_STRING(".bss", section_get_name(bss_sec));
    TEST_ASSERT_EQUAL_STRING(".note", section_get_name(note_sec));
    
    /* Cleanup */
    section_list_destroy(text_sections);
    section_list_destroy(data_sections);
    section_list_destroy(bss_sections);
    section_list_destroy(note_sections);
}

/**
 * @brief Test section with NULL parameters
 */
void test_section_null_parameters(void) {
    /* Test operations with NULL manager */
    TEST_ASSERT_EQUAL_size_t(0, section_manager_get_count(NULL));
    TEST_ASSERT_TRUE(section_manager_is_empty(NULL));
    TEST_ASSERT_EQUAL(SECTION_ID_INVALID, section_manager_find_section(NULL, ".text"));
    TEST_ASSERT_NULL(section_manager_get_section(NULL, 0));
    
    /* Test with NULL name */
    section_id_t id = section_manager_create_section(test_manager, NULL, SECTION_TYPE_TEXT, SECTION_FLAG_EXECUTABLE);
    TEST_ASSERT_EQUAL(SECTION_ID_INVALID, id);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Main test runner function
 */
int main(void) {
    UNITY_BEGIN();
    
    /* Basic functionality tests */
    RUN_TEST(test_section_manager_lifecycle);
    RUN_TEST(test_section_creation_basic);
    RUN_TEST(test_section_retrieval);
    RUN_TEST(test_section_lookup_by_name);
    
    /* Data management tests */
    RUN_TEST(test_section_data_management);
    RUN_TEST(test_section_alignment);
    RUN_TEST(test_section_address_assignment);
    
    /* Advanced functionality tests */
    RUN_TEST(test_section_merging);
    RUN_TEST(test_section_layout);
    RUN_TEST(test_section_iteration);
    RUN_TEST(test_section_filtering);
    
    /* Edge case tests */
    RUN_TEST(test_section_null_parameters);
    
    return UNITY_END();
}
