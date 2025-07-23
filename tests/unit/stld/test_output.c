/**
 * @file test_output.c
 * @brief Unit tests for STLD output generation
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * Comprehensive unit tests for output file generation including binary flat
 * format, executable format, memory map generation, and output file
 * validation. Tests cover different output types and target architectures.
 */

#include "unity.h"
#include "output.h"
#include "section.h"
#include "symbol_table.h"
#include "memory.h"
#include "error.h"
#include <string.h>
#include <stdio.h>

/* Test fixture data */
static output_generator_t* test_generator;
static section_manager_t* test_section_manager;
static symbol_table_t* test_symbol_table;
static memory_pool_t* test_memory_pool;
static error_context_t* test_error_ctx;

/**
 * @brief Setup function called before each test
 */
void setUp(void) {
    test_error_ctx = error_context_create();
    TEST_ASSERT_NOT_NULL(test_error_ctx);
    
    test_memory_pool = memory_pool_create(16384);
    TEST_ASSERT_NOT_NULL(test_memory_pool);
    
    test_section_manager = section_manager_create(test_memory_pool, test_error_ctx);
    TEST_ASSERT_NOT_NULL(test_section_manager);
    
    test_symbol_table = symbol_table_create(test_error_ctx);
    TEST_ASSERT_NOT_NULL(test_symbol_table);
    
    test_generator = output_generator_create(test_section_manager, test_symbol_table, test_error_ctx);
    TEST_ASSERT_NOT_NULL(test_generator);
}

/**
 * @brief Teardown function called after each test
 */
void tearDown(void) {
    if (test_generator != NULL) {
        output_generator_destroy(test_generator);
        test_generator = NULL;
    }
    
    if (test_symbol_table != NULL) {
        symbol_table_destroy(test_symbol_table);
        test_symbol_table = NULL;
    }
    
    if (test_section_manager != NULL) {
        section_manager_destroy(test_section_manager);
        test_section_manager = NULL;
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
 * @brief Test output generator creation and destruction
 */
void test_output_generator_lifecycle(void) {
    output_generator_t* generator = output_generator_create(test_section_manager, 
                                                           test_symbol_table, 
                                                           test_error_ctx);
    TEST_ASSERT_NOT_NULL(generator);
    
    output_generator_destroy(generator);
    
    /* Test with NULL parameters */
    generator = output_generator_create(NULL, test_symbol_table, test_error_ctx);
    TEST_ASSERT_NULL(generator);
    
    generator = output_generator_create(test_section_manager, NULL, test_error_ctx);
    TEST_ASSERT_NULL(generator);
    
    generator = output_generator_create(test_section_manager, test_symbol_table, NULL);
    TEST_ASSERT_NULL(generator);
    
    /* Test destruction with NULL */
    output_generator_destroy(NULL); /* Should not crash */
}

/**
 * @brief Test binary flat output generation
 */
void test_binary_flat_output(void) {
    /* Create test sections with data */
    section_id_t text_section = section_manager_create_section(test_section_manager, ".text",
                                                              SECTION_TYPE_TEXT, SECTION_FLAG_EXECUTABLE);
    section_id_t data_section = section_manager_create_section(test_section_manager, ".data",
                                                              SECTION_TYPE_DATA, SECTION_FLAG_WRITABLE);
    
    /* Add test code and data */
    uint8_t text_data[] = {
        0x55,                   /* push %ebp */
        0x89, 0xe5,            /* mov %esp, %ebp */
        0xb8, 0x42, 0x00, 0x00, 0x00,  /* mov $0x42, %eax */
        0x5d,                   /* pop %ebp */
        0xc3                    /* ret */
    };
    
    uint8_t data_data[] = {
        0x48, 0x65, 0x6c, 0x6c, 0x6f,  /* "Hello" */
        0x00, 0x00, 0x00               /* padding */
    };
    
    section_manager_add_data(test_section_manager, text_section, text_data, sizeof(text_data));
    section_manager_add_data(test_section_manager, data_section, data_data, sizeof(data_data));
    
    /* Assign addresses for flat binary layout */
    section_manager_assign_address(test_section_manager, text_section, 0x1000);
    section_manager_assign_address(test_section_manager, data_section, 0x2000);
    
    /* Configure output for binary flat format */
    output_config_t config = {
        .type = OUTPUT_TYPE_BINARY_FLAT,
        .base_address = 0x1000,
        .entry_point = 0x1000,
        .fill_gaps = true,
        .fill_value = 0x90,  /* NOP instruction */
        .generate_map = true
    };
    
    bool result = output_generator_configure(test_generator, &config);
    TEST_ASSERT_TRUE(result);
    
    /* Generate output to memory buffer */
    size_t output_size = output_generator_calculate_size(test_generator);
    TEST_ASSERT_GREATER_THAN(sizeof(text_data) + sizeof(data_data), output_size);
    
    uint8_t* output_buffer = malloc(output_size);
    TEST_ASSERT_NOT_NULL(output_buffer);
    
    result = output_generator_generate_to_buffer(test_generator, output_buffer, output_size);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    /* Verify output structure */
    /* Text section should be at the beginning (offset 0x1000 - 0x1000 = 0) */
    TEST_ASSERT_EQUAL_MEMORY(text_data, output_buffer, sizeof(text_data));
    
    /* Gap between text and data should be filled with NOPs */
    size_t gap_start = sizeof(text_data);
    size_t gap_size = 0x2000 - 0x1000 - sizeof(text_data);
    for (size_t i = 0; i < gap_size; i++) {
        TEST_ASSERT_EQUAL_HEX8(0x90, output_buffer[gap_start + i]);
    }
    
    /* Data section should follow the gap */
    size_t data_offset = gap_start + gap_size;
    TEST_ASSERT_EQUAL_MEMORY(data_data, output_buffer + data_offset, sizeof(data_data));
    
    free(output_buffer);
}

/**
 * @brief Test executable output generation
 */
void test_executable_output(void) {
    /* Create sections for executable */
    section_id_t text_section = section_manager_create_section(test_section_manager, ".text",
                                                              SECTION_TYPE_TEXT, SECTION_FLAG_EXECUTABLE);
    section_id_t data_section = section_manager_create_section(test_section_manager, ".data",
                                                              SECTION_TYPE_DATA, SECTION_FLAG_WRITABLE);
    
    /* Add entry point symbol */
    symbol_t entry_symbol = {
        .name = "_start",
        .type = SYMBOL_TYPE_FUNCTION,
        .binding = SYMBOL_BINDING_GLOBAL,
        .value = 0x1000,
        .size = 20,
        .section_index = 1  /* Text section */
    };
    
    symbol_handle_t entry_handle = symbol_table_insert(test_symbol_table, &entry_symbol);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, entry_handle);
    
    /* Add simple program */
    uint8_t program_code[] = {
        /* _start: */
        0xb8, 0x01, 0x00, 0x00, 0x00,  /* mov $1, %eax (sys_exit) */
        0xbb, 0x00, 0x00, 0x00, 0x00,  /* mov $0, %ebx (exit code) */
        0xcd, 0x80                      /* int $0x80 (syscall) */
    };
    
    section_manager_add_data(test_section_manager, text_section, program_code, sizeof(program_code));
    section_manager_assign_address(test_section_manager, text_section, 0x8048000);  /* Typical Linux load address */
    
    /* Configure for executable output */
    output_config_t config = {
        .type = OUTPUT_TYPE_EXECUTABLE,
        .target_arch = OUTPUT_ARCH_X86_32,
        .target_os = OUTPUT_OS_LINUX,
        .entry_point = 0x8048000,
        .generate_map = true
    };
    
    bool result = output_generator_configure(test_generator, &config);
    TEST_ASSERT_TRUE(result);
    
    /* Generate executable */
    size_t exe_size = output_generator_calculate_size(test_generator);
    TEST_ASSERT_GREATER_THAN(sizeof(program_code), exe_size);  /* Should include ELF headers */
    
    uint8_t* exe_buffer = malloc(exe_size);
    TEST_ASSERT_NOT_NULL(exe_buffer);
    
    result = output_generator_generate_to_buffer(test_generator, exe_buffer, exe_size);
    TEST_ASSERT_TRUE(result);
    
    /* Verify ELF magic number */
    TEST_ASSERT_EQUAL_HEX8(0x7f, exe_buffer[0]);
    TEST_ASSERT_EQUAL_HEX8('E', exe_buffer[1]);
    TEST_ASSERT_EQUAL_HEX8('L', exe_buffer[2]);
    TEST_ASSERT_EQUAL_HEX8('F', exe_buffer[3]);
    
    /* Verify class (32-bit) */
    TEST_ASSERT_EQUAL_HEX8(1, exe_buffer[4]);  /* ELFCLASS32 */
    
    /* Verify data encoding (little-endian) */
    TEST_ASSERT_EQUAL_HEX8(1, exe_buffer[5]);  /* ELFDATA2LSB */
    
    free(exe_buffer);
}

/**
 * @brief Test memory map generation
 */
void test_memory_map_generation(void) {
    /* Create sections */
    section_id_t text_section = section_manager_create_section(test_section_manager, ".text",
                                                              SECTION_TYPE_TEXT, SECTION_FLAG_EXECUTABLE);
    section_id_t data_section = section_manager_create_section(test_section_manager, ".data",
                                                              SECTION_TYPE_DATA, SECTION_FLAG_WRITABLE);
    section_id_t bss_section = section_manager_create_section(test_section_manager, ".bss",
                                                             SECTION_TYPE_BSS, SECTION_FLAG_WRITABLE);
    
    /* Add symbols */
    symbol_t symbols[] = {
        { .name = "main", .type = SYMBOL_TYPE_FUNCTION, .binding = SYMBOL_BINDING_GLOBAL, .value = 0x1000, .size = 50 },
        { .name = "data_var", .type = SYMBOL_TYPE_OBJECT, .binding = SYMBOL_BINDING_GLOBAL, .value = 0x2000, .size = 4 },
        { .name = "bss_var", .type = SYMBOL_TYPE_OBJECT, .binding = SYMBOL_BINDING_GLOBAL, .value = 0x3000, .size = 100 }
    };
    
    for (size_t i = 0; i < sizeof(symbols) / sizeof(symbols[0]); i++) {
        symbol_handle_t handle = symbol_table_insert(test_symbol_table, &symbols[i]);
        TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, handle);
    }
    
    /* Set up sections */
    uint8_t text_data[64] = {0};
    uint8_t data_data[32] = {0};
    
    section_manager_add_data(test_section_manager, text_section, text_data, sizeof(text_data));
    section_manager_add_data(test_section_manager, data_section, data_data, sizeof(data_data));
    section_manager_set_size(test_section_manager, bss_section, 128);
    
    section_manager_assign_address(test_section_manager, text_section, 0x1000);
    section_manager_assign_address(test_section_manager, data_section, 0x2000);
    section_manager_assign_address(test_section_manager, bss_section, 0x3000);
    
    /* Enable memory map generation */
    output_config_t config = {
        .type = OUTPUT_TYPE_BINARY_FLAT,
        .generate_map = true,
        .map_include_symbols = true,
        .map_include_sections = true
    };
    
    bool result = output_generator_configure(test_generator, &config);
    TEST_ASSERT_TRUE(result);
    
    /* Generate memory map */
    memory_map_t* map = output_generator_generate_memory_map(test_generator);
    TEST_ASSERT_NOT_NULL(map);
    
    /* Verify map contents */
    TEST_ASSERT_EQUAL_size_t(3, memory_map_get_section_count(map));
    TEST_ASSERT_EQUAL_size_t(3, memory_map_get_symbol_count(map));
    
    /* Check section entries */
    const memory_map_section_t* text_map_section = memory_map_find_section(map, ".text");
    const memory_map_section_t* data_map_section = memory_map_find_section(map, ".data");
    const memory_map_section_t* bss_map_section = memory_map_find_section(map, ".bss");
    
    TEST_ASSERT_NOT_NULL(text_map_section);
    TEST_ASSERT_NOT_NULL(data_map_section);
    TEST_ASSERT_NOT_NULL(bss_map_section);
    
    TEST_ASSERT_EQUAL_HEX32(0x1000, text_map_section->start_address);
    TEST_ASSERT_EQUAL_HEX32(0x1000 + sizeof(text_data), text_map_section->end_address);
    
    TEST_ASSERT_EQUAL_HEX32(0x2000, data_map_section->start_address);
    TEST_ASSERT_EQUAL_HEX32(0x2000 + sizeof(data_data), data_map_section->end_address);
    
    TEST_ASSERT_EQUAL_HEX32(0x3000, bss_map_section->start_address);
    TEST_ASSERT_EQUAL_HEX32(0x3000 + 128, bss_map_section->end_address);
    
    /* Check symbol entries */
    const memory_map_symbol_t* main_symbol = memory_map_find_symbol(map, "main");
    const memory_map_symbol_t* data_symbol = memory_map_find_symbol(map, "data_var");
    const memory_map_symbol_t* bss_symbol = memory_map_find_symbol(map, "bss_var");
    
    TEST_ASSERT_NOT_NULL(main_symbol);
    TEST_ASSERT_NOT_NULL(data_symbol);
    TEST_ASSERT_NOT_NULL(bss_symbol);
    
    TEST_ASSERT_EQUAL_HEX32(0x1000, main_symbol->address);
    TEST_ASSERT_EQUAL_HEX32(0x2000, data_symbol->address);
    TEST_ASSERT_EQUAL_HEX32(0x3000, bss_symbol->address);
    
    memory_map_destroy(map);
}

/**
 * @brief Test output file writing
 */
void test_output_file_writing(void) {
    /* Create simple binary content */
    section_id_t section_id = section_manager_create_section(test_section_manager, ".text",
                                                            SECTION_TYPE_TEXT, SECTION_FLAG_EXECUTABLE);
    
    uint8_t test_data[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x21, 0x0a};  /* "Hello!\n" */
    section_manager_add_data(test_section_manager, section_id, test_data, sizeof(test_data));
    section_manager_assign_address(test_section_manager, section_id, 0x1000);
    
    /* Configure for binary output */
    output_config_t config = {
        .type = OUTPUT_TYPE_BINARY_FLAT,
        .base_address = 0x1000
    };
    
    bool result = output_generator_configure(test_generator, &config);
    TEST_ASSERT_TRUE(result);
    
    /* Generate to temporary file */
    const char* temp_filename = "/tmp/test_output.bin";
    result = output_generator_generate_to_file(test_generator, temp_filename);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    /* Verify file was created and has correct content */
    FILE* file = fopen(temp_filename, "rb");
    TEST_ASSERT_NOT_NULL(file);
    
    /* Check file size */
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    TEST_ASSERT_EQUAL_INT(sizeof(test_data), file_size);
    
    /* Check file content */
    fseek(file, 0, SEEK_SET);
    uint8_t read_data[sizeof(test_data)];
    size_t bytes_read = fread(read_data, 1, sizeof(read_data), file);
    TEST_ASSERT_EQUAL_size_t(sizeof(test_data), bytes_read);
    TEST_ASSERT_EQUAL_MEMORY(test_data, read_data, sizeof(test_data));
    
    fclose(file);
    
    /* Clean up temporary file */
    remove(temp_filename);
}

/**
 * @brief Test output size calculation
 */
void test_output_size_calculation(void) {
    /* Create sections with known sizes */
    section_id_t text_section = section_manager_create_section(test_section_manager, ".text",
                                                              SECTION_TYPE_TEXT, SECTION_FLAG_EXECUTABLE);
    section_id_t data_section = section_manager_create_section(test_section_manager, ".data",
                                                              SECTION_TYPE_DATA, SECTION_FLAG_WRITABLE);
    
    uint8_t text_data[100];
    uint8_t data_data[50];
    memset(text_data, 0x90, sizeof(text_data));
    memset(data_data, 0x00, sizeof(data_data));
    
    section_manager_add_data(test_section_manager, text_section, text_data, sizeof(text_data));
    section_manager_add_data(test_section_manager, data_section, data_data, sizeof(data_data));
    
    /* Test binary flat output size (contiguous) */
    section_manager_assign_address(test_section_manager, text_section, 0x1000);
    section_manager_assign_address(test_section_manager, data_section, 0x1100);  /* Right after text */
    
    output_config_t config = {
        .type = OUTPUT_TYPE_BINARY_FLAT,
        .base_address = 0x1000,
        .fill_gaps = false
    };
    
    bool result = output_generator_configure(test_generator, &config);
    TEST_ASSERT_TRUE(result);
    
    size_t calculated_size = output_generator_calculate_size(test_generator);
    size_t expected_size = sizeof(text_data) + sizeof(data_data);
    TEST_ASSERT_EQUAL_size_t(expected_size, calculated_size);
    
    /* Test with gap filling */
    section_manager_assign_address(test_section_manager, data_section, 0x2000);  /* Create gap */
    
    config.fill_gaps = true;
    result = output_generator_configure(test_generator, &config);
    TEST_ASSERT_TRUE(result);
    
    calculated_size = output_generator_calculate_size(test_generator);
    expected_size = (0x2000 - 0x1000) + sizeof(data_data);  /* From base to end of data section */
    TEST_ASSERT_EQUAL_size_t(expected_size, calculated_size);
}

/**
 * @brief Test output configuration validation
 */
void test_output_configuration_validation(void) {
    output_config_t config;
    
    /* Test valid configuration */
    config = (output_config_t) {
        .type = OUTPUT_TYPE_BINARY_FLAT,
        .base_address = 0x1000,
        .entry_point = 0x1000,
        .fill_gaps = false
    };
    
    bool result = output_generator_configure(test_generator, &config);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    /* Test invalid output type */
    config.type = (output_type_t)999;
    result = output_generator_configure(test_generator, &config);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
    
    error_context_clear(test_error_ctx);
    
    /* Test NULL configuration */
    result = output_generator_configure(test_generator, NULL);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Test cross-reference generation
 */
void test_cross_reference_generation(void) {
    /* Create symbols with cross-references */
    symbol_t symbols[] = {
        { .name = "main", .type = SYMBOL_TYPE_FUNCTION, .binding = SYMBOL_BINDING_GLOBAL, .value = 0x1000 },
        { .name = "helper", .type = SYMBOL_TYPE_FUNCTION, .binding = SYMBOL_BINDING_LOCAL, .value = 0x1100 },
        { .name = "global_var", .type = SYMBOL_TYPE_OBJECT, .binding = SYMBOL_BINDING_GLOBAL, .value = 0x2000 }
    };
    
    for (size_t i = 0; i < sizeof(symbols) / sizeof(symbols[0]); i++) {
        symbol_handle_t handle = symbol_table_insert(test_symbol_table, &symbols[i]);
        TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, handle);
    }
    
    /* Enable cross-reference generation */
    output_config_t config = {
        .type = OUTPUT_TYPE_BINARY_FLAT,
        .generate_cross_ref = true
    };
    
    bool result = output_generator_configure(test_generator, &config);
    TEST_ASSERT_TRUE(result);
    
    /* Generate cross-reference table */
    cross_reference_t* xref = output_generator_generate_cross_reference(test_generator);
    TEST_ASSERT_NOT_NULL(xref);
    
    /* Verify cross-reference contents */
    TEST_ASSERT_EQUAL_size_t(3, cross_reference_get_symbol_count(xref));
    
    const cross_ref_entry_t* main_entry = cross_reference_find_symbol(xref, "main");
    const cross_ref_entry_t* helper_entry = cross_reference_find_symbol(xref, "helper");
    const cross_ref_entry_t* var_entry = cross_reference_find_symbol(xref, "global_var");
    
    TEST_ASSERT_NOT_NULL(main_entry);
    TEST_ASSERT_NOT_NULL(helper_entry);
    TEST_ASSERT_NOT_NULL(var_entry);
    
    TEST_ASSERT_EQUAL_HEX32(0x1000, main_entry->address);
    TEST_ASSERT_EQUAL_HEX32(0x1100, helper_entry->address);
    TEST_ASSERT_EQUAL_HEX32(0x2000, var_entry->address);
    
    cross_reference_destroy(xref);
}

/**
 * @brief Test output with NULL parameters
 */
void test_output_null_parameters(void) {
    /* Test operations with NULL generator */
    TEST_ASSERT_FALSE(output_generator_configure(NULL, NULL));
    TEST_ASSERT_EQUAL_size_t(0, output_generator_calculate_size(NULL));
    TEST_ASSERT_FALSE(output_generator_generate_to_file(NULL, "test.bin"));
    
    /* Test with NULL filename */
    bool result = output_generator_generate_to_file(test_generator, NULL);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Main test runner function
 */
int main(void) {
    UNITY_BEGIN();
    
    /* Basic functionality tests */
    RUN_TEST(test_output_generator_lifecycle);
    RUN_TEST(test_output_configuration_validation);
    
    /* Output format tests */
    RUN_TEST(test_binary_flat_output);
    RUN_TEST(test_executable_output);
    
    /* Output features tests */
    RUN_TEST(test_memory_map_generation);
    RUN_TEST(test_cross_reference_generation);
    
    /* File I/O tests */
    RUN_TEST(test_output_file_writing);
    RUN_TEST(test_output_size_calculation);
    
    /* Edge case tests */
    RUN_TEST(test_output_null_parameters);
    
    return UNITY_END();
}
