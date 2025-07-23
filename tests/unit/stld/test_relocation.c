/**
 * @file test_relocation.c
 * @brief Unit tests for STLD relocation engine
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * Comprehensive unit tests for relocation processing including relocation
 * entry parsing, address calculation, relocation application, and various
 * relocation types for x86_32 architecture.
 */

#include "unity.h"
#include "relocation.h"
#include "symbol_table.h"
#include "section.h"
#include "memory.h"
#include "error.h"
#include <string.h>

/* Test fixture data */
static relocation_engine_t* test_engine;
static symbol_table_t* test_symbol_table;
static section_manager_t* test_section_manager;
static memory_pool_t* test_memory_pool;
static error_context_t* test_error_ctx;

/**
 * @brief Setup function called before each test
 */
void setUp(void) {
    test_error_ctx = error_context_create();
    TEST_ASSERT_NOT_NULL(test_error_ctx);
    
    test_memory_pool = memory_pool_create(8192);
    TEST_ASSERT_NOT_NULL(test_memory_pool);
    
    test_symbol_table = symbol_table_create(test_error_ctx);
    TEST_ASSERT_NOT_NULL(test_symbol_table);
    
    test_section_manager = section_manager_create(test_memory_pool, test_error_ctx);
    TEST_ASSERT_NOT_NULL(test_section_manager);
    
    test_engine = relocation_engine_create(test_symbol_table, test_section_manager, test_error_ctx);
    TEST_ASSERT_NOT_NULL(test_engine);
}

/**
 * @brief Teardown function called after each test
 */
void tearDown(void) {
    if (test_engine != NULL) {
        relocation_engine_destroy(test_engine);
        test_engine = NULL;
    }
    
    if (test_section_manager != NULL) {
        section_manager_destroy(test_section_manager);
        test_section_manager = NULL;
    }
    
    if (test_symbol_table != NULL) {
        symbol_table_destroy(test_symbol_table);
        test_symbol_table = NULL;
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
 * @brief Test relocation engine creation and destruction
 */
void test_relocation_engine_lifecycle(void) {
    relocation_engine_t* engine = relocation_engine_create(test_symbol_table, 
                                                          test_section_manager, 
                                                          test_error_ctx);
    TEST_ASSERT_NOT_NULL(engine);
    
    /* Initial state */
    TEST_ASSERT_EQUAL_size_t(0, relocation_engine_get_count(engine));
    TEST_ASSERT_FALSE(relocation_engine_has_unresolved(engine));
    
    relocation_engine_destroy(engine);
    
    /* Test with NULL parameters */
    engine = relocation_engine_create(NULL, test_section_manager, test_error_ctx);
    TEST_ASSERT_NULL(engine);
    
    engine = relocation_engine_create(test_symbol_table, NULL, test_error_ctx);
    TEST_ASSERT_NULL(engine);
    
    engine = relocation_engine_create(test_symbol_table, test_section_manager, NULL);
    TEST_ASSERT_NULL(engine);
    
    /* Test destruction with NULL */
    relocation_engine_destroy(NULL); /* Should not crash */
}

/**
 * @brief Test basic relocation entry creation
 */
void test_relocation_entry_creation(void) {
    /* Create test section and symbol */
    section_id_t section_id = section_manager_create_section(test_section_manager, ".text", 
                                                            SECTION_TYPE_TEXT, SECTION_FLAG_EXECUTABLE);
    TEST_ASSERT_NOT_EQUAL(SECTION_ID_INVALID, section_id);
    
    symbol_t symbol = {
        .name = "target_func",
        .type = SYMBOL_TYPE_FUNCTION,
        .binding = SYMBOL_BINDING_GLOBAL,
        .value = 0x2000,
        .size = 0x100
    };
    
    symbol_handle_t symbol_handle = symbol_table_insert(test_symbol_table, &symbol);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, symbol_handle);
    
    /* Create relocation entry */
    relocation_entry_t reloc = {
        .offset = 0x10,
        .type = RELOC_TYPE_PC32,
        .symbol_handle = symbol_handle,
        .addend = 0,
        .section_id = section_id
    };
    
    relocation_id_t reloc_id = relocation_engine_add_entry(test_engine, &reloc);
    TEST_ASSERT_NOT_EQUAL(RELOCATION_ID_INVALID, reloc_id);
    
    /* Verify engine state */
    TEST_ASSERT_EQUAL_size_t(1, relocation_engine_get_count(test_engine));
    TEST_ASSERT_TRUE(relocation_engine_has_unresolved(test_engine));
    
    /* Verify no errors occurred */
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Test PC-relative relocation calculation
 */
void test_pc_relative_relocation(void) {
    /* Set up test scenario:
     * - Text section at 0x1000 with code
     * - Call instruction at offset 0x10 that needs to call function at 0x2000
     */
    
    /* Create and set up text section */
    section_id_t text_section = section_manager_create_section(test_section_manager, ".text",
                                                              SECTION_TYPE_TEXT, SECTION_FLAG_EXECUTABLE);
    section_manager_assign_address(test_section_manager, text_section, 0x1000);
    
    /* Add some code data */
    uint8_t code_data[32] = {
        0x55,                   /* push %ebp */
        0x89, 0xe5,            /* mov %esp, %ebp */
        0xe8, 0x00, 0x00, 0x00, 0x00,  /* call 0x0 (placeholder - offset 3-6) */
        0x5d,                   /* pop %ebp */
        0xc3                    /* ret */
    };
    section_manager_add_data(test_section_manager, text_section, code_data, sizeof(code_data));
    
    /* Create target symbol */
    symbol_t target_symbol = {
        .name = "target_function",
        .type = SYMBOL_TYPE_FUNCTION,
        .binding = SYMBOL_BINDING_GLOBAL,
        .value = 0x2000,
        .size = 0x100
    };
    
    symbol_handle_t target_handle = symbol_table_insert(test_symbol_table, &target_symbol);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, target_handle);
    
    /* Create PC-relative relocation for the call instruction */
    relocation_entry_t reloc = {
        .offset = 4,  /* Offset of the displacement in call instruction */
        .type = RELOC_TYPE_PC32,
        .symbol_handle = target_handle,
        .addend = -4,  /* Account for instruction size */
        .section_id = text_section
    };
    
    relocation_id_t reloc_id = relocation_engine_add_entry(test_engine, &reloc);
    TEST_ASSERT_NOT_EQUAL(RELOCATION_ID_INVALID, reloc_id);
    
    /* Process relocations */
    bool result = relocation_engine_process_all(test_engine);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    /* Verify relocation was applied correctly */
    const section_t* section = section_manager_get_section(test_section_manager, text_section);
    const uint8_t* updated_code = section_get_data(section);
    
    /* Calculate expected displacement:
     * target_address - (instruction_address + instruction_size)
     * 0x2000 - (0x1000 + 4 + 4) = 0x2000 - 0x1008 = 0xFF8
     */
    uint32_t expected_displacement = 0x2000 - (0x1000 + 4 + 4);
    
    /* Extract displacement from code (little-endian) */
    uint32_t actual_displacement = 
        (uint32_t)updated_code[4] |
        ((uint32_t)updated_code[5] << 8) |
        ((uint32_t)updated_code[6] << 16) |
        ((uint32_t)updated_code[7] << 24);
    
    TEST_ASSERT_EQUAL_HEX32(expected_displacement, actual_displacement);
    
    /* Engine should have no unresolved relocations */
    TEST_ASSERT_FALSE(relocation_engine_has_unresolved(test_engine));
}

/**
 * @brief Test absolute address relocation
 */
void test_absolute_relocation(void) {
    /* Set up data section with pointer that needs absolute address */
    section_id_t data_section = section_manager_create_section(test_section_manager, ".data",
                                                              SECTION_TYPE_DATA, SECTION_FLAG_WRITABLE);
    section_manager_assign_address(test_section_manager, data_section, 0x3000);
    
    /* Data with placeholder pointer */
    uint8_t data[] = {
        0x00, 0x00, 0x00, 0x00,  /* Pointer placeholder (offset 0) */
        0x12, 0x34, 0x56, 0x78   /* Other data */
    };
    section_manager_add_data(test_section_manager, data_section, data, sizeof(data));
    
    /* Create target symbol */
    symbol_t target_symbol = {
        .name = "global_variable",
        .type = SYMBOL_TYPE_OBJECT,
        .binding = SYMBOL_BINDING_GLOBAL,
        .value = 0x4000,
        .size = 4
    };
    
    symbol_handle_t target_handle = symbol_table_insert(test_symbol_table, &target_symbol);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, target_handle);
    
    /* Create absolute relocation */
    relocation_entry_t reloc = {
        .offset = 0,  /* Pointer at beginning of data */
        .type = RELOC_TYPE_ABS32,
        .symbol_handle = target_handle,
        .addend = 0,
        .section_id = data_section
    };
    
    relocation_id_t reloc_id = relocation_engine_add_entry(test_engine, &reloc);
    TEST_ASSERT_NOT_EQUAL(RELOCATION_ID_INVALID, reloc_id);
    
    /* Process relocations */
    bool result = relocation_engine_process_all(test_engine);
    TEST_ASSERT_TRUE(result);
    
    /* Verify absolute address was written */
    const section_t* section = section_manager_get_section(test_section_manager, data_section);
    const uint8_t* updated_data = section_get_data(section);
    
    uint32_t actual_address = 
        (uint32_t)updated_data[0] |
        ((uint32_t)updated_data[1] << 8) |
        ((uint32_t)updated_data[2] << 16) |
        ((uint32_t)updated_data[3] << 24);
    
    TEST_ASSERT_EQUAL_HEX32(0x4000, actual_address);
    
    /* Other data should be unchanged */
    TEST_ASSERT_EQUAL_HEX8(0x12, updated_data[4]);
    TEST_ASSERT_EQUAL_HEX8(0x34, updated_data[5]);
    TEST_ASSERT_EQUAL_HEX8(0x56, updated_data[6]);
    TEST_ASSERT_EQUAL_HEX8(0x78, updated_data[7]);
}

/**
 * @brief Test relocation with addend
 */
void test_relocation_with_addend(void) {
    /* Create section and symbol */
    section_id_t section_id = section_manager_create_section(test_section_manager, ".data",
                                                            SECTION_TYPE_DATA, SECTION_FLAG_WRITABLE);
    section_manager_assign_address(test_section_manager, section_id, 0x2000);
    
    uint8_t data[8] = {0};
    section_manager_add_data(test_section_manager, section_id, data, sizeof(data));
    
    symbol_t symbol = {
        .name = "array_base",
        .type = SYMBOL_TYPE_OBJECT,
        .binding = SYMBOL_BINDING_GLOBAL,
        .value = 0x5000,
        .size = 100
    };
    
    symbol_handle_t symbol_handle = symbol_table_insert(test_symbol_table, &symbol);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, symbol_handle);
    
    /* Create relocation with addend (pointer to array[10]) */
    relocation_entry_t reloc = {
        .offset = 0,
        .type = RELOC_TYPE_ABS32,
        .symbol_handle = symbol_handle,
        .addend = 40,  /* 10 * sizeof(int) = 10 * 4 = 40 */
        .section_id = section_id
    };
    
    relocation_id_t reloc_id = relocation_engine_add_entry(test_engine, &reloc);
    TEST_ASSERT_NOT_EQUAL(RELOCATION_ID_INVALID, reloc_id);
    
    /* Process relocations */
    bool result = relocation_engine_process_all(test_engine);
    TEST_ASSERT_TRUE(result);
    
    /* Verify address with addend */
    const section_t* section = section_manager_get_section(test_section_manager, section_id);
    const uint8_t* updated_data = section_get_data(section);
    
    uint32_t actual_address = 
        (uint32_t)updated_data[0] |
        ((uint32_t)updated_data[1] << 8) |
        ((uint32_t)updated_data[2] << 16) |
        ((uint32_t)updated_data[3] << 24);
    
    uint32_t expected_address = 0x5000 + 40;
    TEST_ASSERT_EQUAL_HEX32(expected_address, actual_address);
}

/**
 * @brief Test unresolved symbol handling
 */
void test_unresolved_symbol_handling(void) {
    /* Create section */
    section_id_t section_id = section_manager_create_section(test_section_manager, ".text",
                                                            SECTION_TYPE_TEXT, SECTION_FLAG_EXECUTABLE);
    
    /* Create undefined symbol */
    symbol_t undefined_symbol = {
        .name = "undefined_function",
        .type = SYMBOL_TYPE_FUNCTION,
        .binding = SYMBOL_BINDING_GLOBAL,
        .value = 0,  /* Undefined */
        .size = 0,
        .section_index = SECTION_INDEX_UNDEFINED
    };
    
    symbol_handle_t undefined_handle = symbol_table_insert(test_symbol_table, &undefined_symbol);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, undefined_handle);
    
    /* Create relocation to undefined symbol */
    relocation_entry_t reloc = {
        .offset = 0,
        .type = RELOC_TYPE_PC32,
        .symbol_handle = undefined_handle,
        .addend = 0,
        .section_id = section_id
    };
    
    relocation_id_t reloc_id = relocation_engine_add_entry(test_engine, &reloc);
    TEST_ASSERT_NOT_EQUAL(RELOCATION_ID_INVALID, reloc_id);
    
    /* Processing should fail with unresolved symbol */
    bool result = relocation_engine_process_all(test_engine);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
    
    /* Should still have unresolved relocations */
    TEST_ASSERT_TRUE(relocation_engine_has_unresolved(test_engine));
    
    /* Get list of unresolved symbols */
    symbol_list_t* unresolved = relocation_engine_get_unresolved_symbols(test_engine);
    TEST_ASSERT_NOT_NULL(unresolved);
    TEST_ASSERT_EQUAL_size_t(1, symbol_list_size(unresolved));
    
    const symbol_t* unresolved_symbol = symbol_list_get(unresolved, 0);
    TEST_ASSERT_EQUAL_STRING("undefined_function", unresolved_symbol->name);
    
    symbol_list_destroy(unresolved);
}

/**
 * @brief Test different relocation types
 */
void test_different_relocation_types(void) {
    /* Create test section */
    section_id_t section_id = section_manager_create_section(test_section_manager, ".mixed",
                                                            SECTION_TYPE_PROGBITS, SECTION_FLAG_READABLE);
    section_manager_assign_address(test_section_manager, section_id, 0x1000);
    
    /* Add data with space for different relocation types */
    uint8_t data[32] = {0};
    section_manager_add_data(test_section_manager, section_id, data, sizeof(data));
    
    /* Create target symbol */
    symbol_t target = {
        .name = "target",
        .type = SYMBOL_TYPE_OBJECT,
        .binding = SYMBOL_BINDING_GLOBAL,
        .value = 0x8000,
        .size = 4
    };
    
    symbol_handle_t target_handle = symbol_table_insert(test_symbol_table, &target);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, target_handle);
    
    /* Test different relocation types */
    relocation_entry_t relocations[] = {
        /* ABS32 at offset 0 */
        { .offset = 0, .type = RELOC_TYPE_ABS32, .symbol_handle = target_handle, .addend = 0, .section_id = section_id },
        /* ABS16 at offset 8 */
        { .offset = 8, .type = RELOC_TYPE_ABS16, .symbol_handle = target_handle, .addend = 0, .section_id = section_id },
        /* ABS8 at offset 16 */
        { .offset = 16, .type = RELOC_TYPE_ABS8, .symbol_handle = target_handle, .addend = 0, .section_id = section_id }
    };
    
    /* Add all relocations */
    for (size_t i = 0; i < sizeof(relocations) / sizeof(relocations[0]); i++) {
        relocation_id_t id = relocation_engine_add_entry(test_engine, &relocations[i]);
        TEST_ASSERT_NOT_EQUAL(RELOCATION_ID_INVALID, id);
    }
    
    /* Process all relocations */
    bool result = relocation_engine_process_all(test_engine);
    TEST_ASSERT_TRUE(result);
    
    /* Verify results */
    const section_t* section = section_manager_get_section(test_section_manager, section_id);
    const uint8_t* updated_data = section_get_data(section);
    
    /* Check ABS32 (32-bit absolute address) */
    uint32_t abs32_value = 
        (uint32_t)updated_data[0] |
        ((uint32_t)updated_data[1] << 8) |
        ((uint32_t)updated_data[2] << 16) |
        ((uint32_t)updated_data[3] << 24);
    TEST_ASSERT_EQUAL_HEX32(0x8000, abs32_value);
    
    /* Check ABS16 (16-bit absolute address, truncated) */
    uint16_t abs16_value = 
        (uint16_t)updated_data[8] |
        ((uint16_t)updated_data[9] << 8);
    TEST_ASSERT_EQUAL_HEX16(0x8000 & 0xFFFF, abs16_value);
    
    /* Check ABS8 (8-bit absolute address, truncated) */
    uint8_t abs8_value = updated_data[16];
    TEST_ASSERT_EQUAL_HEX8(0x8000 & 0xFF, abs8_value);
}

/**
 * @brief Test relocation overflow detection
 */
void test_relocation_overflow_detection(void) {
    /* Create section */
    section_id_t section_id = section_manager_create_section(test_section_manager, ".data",
                                                            SECTION_TYPE_DATA, SECTION_FLAG_WRITABLE);
    section_manager_assign_address(test_section_manager, section_id, 0x1000);
    
    uint8_t data[4] = {0};
    section_manager_add_data(test_section_manager, section_id, data, sizeof(data));
    
    /* Create symbol with value that will overflow 16-bit relocation */
    symbol_t large_symbol = {
        .name = "large_address",
        .type = SYMBOL_TYPE_OBJECT,
        .binding = SYMBOL_BINDING_GLOBAL,
        .value = 0x12345678,  /* Won't fit in 16 bits */
        .size = 4
    };
    
    symbol_handle_t large_handle = symbol_table_insert(test_symbol_table, &large_symbol);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, large_handle);
    
    /* Create 16-bit relocation that will overflow */
    relocation_entry_t reloc = {
        .offset = 0,
        .type = RELOC_TYPE_ABS16,
        .symbol_handle = large_handle,
        .addend = 0,
        .section_id = section_id
    };
    
    relocation_id_t reloc_id = relocation_engine_add_entry(test_engine, &reloc);
    TEST_ASSERT_NOT_EQUAL(RELOCATION_ID_INVALID, reloc_id);
    
    /* Processing should detect overflow */
    bool result = relocation_engine_process_all(test_engine);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
    
    /* Error message should mention overflow */
    const char* error_msg = error_context_get_message(test_error_ctx);
    TEST_ASSERT_NOT_NULL(error_msg);
    TEST_ASSERT_TRUE(strstr(error_msg, "overflow") != NULL || strstr(error_msg, "range") != NULL);
}

/**
 * @brief Test relocation statistics and reporting
 */
void test_relocation_statistics(void) {
    /* Create multiple sections and symbols */
    section_id_t text_section = section_manager_create_section(test_section_manager, ".text", SECTION_TYPE_TEXT, SECTION_FLAG_EXECUTABLE);
    section_id_t data_section = section_manager_create_section(test_section_manager, ".data", SECTION_TYPE_DATA, SECTION_FLAG_WRITABLE);
    
    symbol_t symbols[] = {
        { .name = "func1", .type = SYMBOL_TYPE_FUNCTION, .binding = SYMBOL_BINDING_GLOBAL, .value = 0x1000 },
        { .name = "func2", .type = SYMBOL_TYPE_FUNCTION, .binding = SYMBOL_BINDING_GLOBAL, .value = 0x2000 },
        { .name = "var1", .type = SYMBOL_TYPE_OBJECT, .binding = SYMBOL_BINDING_GLOBAL, .value = 0x3000 }
    };
    
    symbol_handle_t handles[sizeof(symbols) / sizeof(symbols[0])];
    for (size_t i = 0; i < sizeof(symbols) / sizeof(symbols[0]); i++) {
        handles[i] = symbol_table_insert(test_symbol_table, &symbols[i]);
        TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, handles[i]);
    }
    
    /* Create multiple relocations */
    relocation_entry_t relocations[] = {
        { .offset = 0, .type = RELOC_TYPE_PC32, .symbol_handle = handles[0], .section_id = text_section },
        { .offset = 8, .type = RELOC_TYPE_PC32, .symbol_handle = handles[1], .section_id = text_section },
        { .offset = 0, .type = RELOC_TYPE_ABS32, .symbol_handle = handles[2], .section_id = data_section },
        { .offset = 4, .type = RELOC_TYPE_ABS32, .symbol_handle = handles[0], .section_id = data_section }
    };
    
    for (size_t i = 0; i < sizeof(relocations) / sizeof(relocations[0]); i++) {
        relocation_id_t id = relocation_engine_add_entry(test_engine, &relocations[i]);
        TEST_ASSERT_NOT_EQUAL(RELOCATION_ID_INVALID, id);
    }
    
    /* Get statistics before processing */
    relocation_stats_t stats = relocation_engine_get_statistics(test_engine);
    TEST_ASSERT_EQUAL_size_t(4, stats.total_relocations);
    TEST_ASSERT_EQUAL_size_t(4, stats.unresolved_relocations);
    TEST_ASSERT_EQUAL_size_t(0, stats.resolved_relocations);
    TEST_ASSERT_EQUAL_size_t(2, stats.pc_relative_count);
    TEST_ASSERT_EQUAL_size_t(2, stats.absolute_count);
    
    /* Process relocations and get updated statistics */
    section_manager_assign_address(test_section_manager, text_section, 0x1000);
    section_manager_assign_address(test_section_manager, data_section, 0x4000);
    
    uint8_t dummy_data[32] = {0};
    section_manager_add_data(test_section_manager, text_section, dummy_data, 16);
    section_manager_add_data(test_section_manager, data_section, dummy_data, 16);
    
    bool result = relocation_engine_process_all(test_engine);
    TEST_ASSERT_TRUE(result);
    
    stats = relocation_engine_get_statistics(test_engine);
    TEST_ASSERT_EQUAL_size_t(4, stats.total_relocations);
    TEST_ASSERT_EQUAL_size_t(0, stats.unresolved_relocations);
    TEST_ASSERT_EQUAL_size_t(4, stats.resolved_relocations);
}

/**
 * @brief Test relocation with NULL parameters
 */
void test_relocation_null_parameters(void) {
    /* Test operations with NULL engine */
    TEST_ASSERT_EQUAL_size_t(0, relocation_engine_get_count(NULL));
    TEST_ASSERT_FALSE(relocation_engine_has_unresolved(NULL));
    TEST_ASSERT_FALSE(relocation_engine_process_all(NULL));
    
    /* Test with NULL relocation entry */
    relocation_id_t id = relocation_engine_add_entry(test_engine, NULL);
    TEST_ASSERT_EQUAL(RELOCATION_ID_INVALID, id);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Main test runner function
 */
int main(void) {
    UNITY_BEGIN();
    
    /* Basic functionality tests */
    RUN_TEST(test_relocation_engine_lifecycle);
    RUN_TEST(test_relocation_entry_creation);
    
    /* Relocation type tests */
    RUN_TEST(test_pc_relative_relocation);
    RUN_TEST(test_absolute_relocation);
    RUN_TEST(test_relocation_with_addend);
    RUN_TEST(test_different_relocation_types);
    
    /* Error handling tests */
    RUN_TEST(test_unresolved_symbol_handling);
    RUN_TEST(test_relocation_overflow_detection);
    
    /* Advanced functionality tests */
    RUN_TEST(test_relocation_statistics);
    
    /* Edge case tests */
    RUN_TEST(test_relocation_null_parameters);
    
    return UNITY_END();
}
