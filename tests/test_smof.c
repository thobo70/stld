/* tests/test_smof.c */
#include "unity.h"
#include "smof.h"
#include <stdio.h>
#include <string.h>

/**
 * @file test_smof.c  
 * @brief Unit tests for SMOF format implementation
 * @details Tests SMOF header validation, structure layout, and utility functions
 */

/* Function prototypes for C90 compliance */
void test_smof_validate_header_valid(void);
void test_smof_validate_header_invalid_magic(void);
void test_smof_validate_header_invalid_version(void);
void test_smof_validate_header_null_header(void);
void test_smof_get_header_size(void);
void test_smof_header_alignment(void);
void test_smof_magic_constant(void);
void test_smof_section_type_values(void);
void test_smof_symbol_binding_values(void);
void test_smof_symbol_type_values(void);
void test_smof_symbol_utility_functions(void);
void test_smof_relocation_utility_functions(void);
int test_smof_main(void);

/* Test data */
static const smof_header_t valid_header = {
    .magic = SMOF_MAGIC,
    .version = SMOF_VERSION,
    .flags = SMOF_FLAG_LITTLE_ENDIAN,
    .entry_point = 0x1000,
    .section_count = 2,
    .symbol_count = 5,
    .section_table_offset = 32,
    .symbol_table_offset = 112,
    .string_table_offset = 192,
    .checksum = 0
};

void setUp(void) {
    /* Setup before each test */
}

void tearDown(void) {
    /* Cleanup after each test */
}

void test_smof_validate_header_valid(void) {
    TEST_ASSERT_TRUE(smof_validate_header(&valid_header));
    TEST_ASSERT_TRUE(smof_header_is_valid(&valid_header));
}

void test_smof_validate_header_invalid_magic(void) {
    smof_header_t header = valid_header;
    header.magic = 0x12345678;
    
    TEST_ASSERT_FALSE(smof_validate_header(&header));
    TEST_ASSERT_FALSE(smof_header_is_valid(&header));
}

void test_smof_validate_header_invalid_version(void) {
    smof_header_t header = valid_header;
    header.version = 99;
    
    TEST_ASSERT_FALSE(smof_validate_header(&header));
    TEST_ASSERT_FALSE(smof_header_is_valid(&header));
}

void test_smof_validate_header_null_header(void) {
    TEST_ASSERT_FALSE(smof_validate_header(NULL));
    TEST_ASSERT_FALSE(smof_header_is_valid(NULL));
}

void test_smof_get_header_size(void) {
    size_t size = sizeof(smof_header_t);
    
    TEST_ASSERT_EQUAL_UINT(32, (uint32_t)size);
}

void test_smof_header_alignment(void) {
    /* Test structure member offsets and alignment */
    TEST_ASSERT_EQUAL_UINT(0, offsetof(smof_header_t, magic));
    TEST_ASSERT_EQUAL_UINT(4, offsetof(smof_header_t, version));
    TEST_ASSERT_EQUAL_UINT(6, offsetof(smof_header_t, flags));
    TEST_ASSERT_EQUAL_UINT(8, offsetof(smof_header_t, entry_point));
    TEST_ASSERT_EQUAL_UINT(12, offsetof(smof_header_t, section_count));
    TEST_ASSERT_EQUAL_UINT(14, offsetof(smof_header_t, symbol_count));
}

void test_smof_magic_constant(void) {
    /* Test magic constant value */
    TEST_ASSERT_EQUAL_HEX32(0x534D4F46U, SMOF_MAGIC);
}

void test_smof_section_type_values(void) {
    /* Test section type enumeration values */
    TEST_ASSERT_EQUAL_UINT(0, SMOF_SECTION_NULL);
    TEST_ASSERT_EQUAL_UINT(1, SMOF_SECTION_PROGBITS);
    TEST_ASSERT_EQUAL_UINT(2, SMOF_SECTION_SYMTAB);
    TEST_ASSERT_EQUAL_UINT(3, SMOF_SECTION_STRTAB);
    TEST_ASSERT_EQUAL_UINT(4, SMOF_SECTION_RELA);
    TEST_ASSERT_EQUAL_UINT(9, SMOF_SECTION_REL);
}

void test_smof_symbol_binding_values(void) {
    /* Test symbol binding enumeration values */
    TEST_ASSERT_EQUAL_UINT(0, SMOF_STB_LOCAL);
    TEST_ASSERT_EQUAL_UINT(1, SMOF_STB_GLOBAL);
    TEST_ASSERT_EQUAL_UINT(2, SMOF_STB_WEAK);
}

void test_smof_symbol_type_values(void) {
    /* Test symbol type enumeration values */
    TEST_ASSERT_EQUAL_UINT(0, SMOF_STT_NOTYPE);
    TEST_ASSERT_EQUAL_UINT(1, SMOF_STT_OBJECT);
    TEST_ASSERT_EQUAL_UINT(2, SMOF_STT_FUNC);
    TEST_ASSERT_EQUAL_UINT(3, SMOF_STT_SECTION);
    TEST_ASSERT_EQUAL_UINT(4, SMOF_STT_FILE);
}

void test_smof_symbol_utility_functions(void) {
    smof_symbol_t symbol;
    uint8_t info;
    
    /* Test symbol info encoding/decoding */
    info = smof_symbol_make_info(SMOF_STB_GLOBAL, SMOF_STT_FUNC);
    symbol.info = info;
    
    TEST_ASSERT_EQUAL_UINT((uint32_t)SMOF_STB_GLOBAL, (uint32_t)smof_symbol_get_binding(&symbol));
    TEST_ASSERT_EQUAL_UINT((uint32_t)SMOF_STT_FUNC, (uint32_t)smof_symbol_get_type(&symbol));
    
    /* Test different combinations */
    info = smof_symbol_make_info(SMOF_STB_LOCAL, SMOF_STT_OBJECT);
    symbol.info = info;
    
    TEST_ASSERT_EQUAL_UINT((uint32_t)SMOF_STB_LOCAL, (uint32_t)smof_symbol_get_binding(&symbol));
    TEST_ASSERT_EQUAL_UINT((uint32_t)SMOF_STT_OBJECT, (uint32_t)smof_symbol_get_type(&symbol));
}

void test_smof_relocation_utility_functions(void) {
    uint32_t info;
    uint32_t symbol_index = 42;
    smof_relocation_type_t rel_type = SMOF_R_32;
    
    /* Test relocation info encoding/decoding */
    info = smof_relocation_make_info(symbol_index, rel_type);
    
    TEST_ASSERT_EQUAL_UINT(symbol_index, smof_relocation_get_symbol(info));
    TEST_ASSERT_EQUAL_UINT((uint32_t)rel_type, (uint32_t)smof_relocation_get_type(info));
    
    /* Test different values */
    symbol_index = 255;
    rel_type = SMOF_R_PC16;
    info = smof_relocation_make_info(symbol_index, rel_type);
    
    TEST_ASSERT_EQUAL_UINT(symbol_index, smof_relocation_get_symbol(info));
    TEST_ASSERT_EQUAL_UINT((uint32_t)rel_type, (uint32_t)smof_relocation_get_type(info));
}

int test_smof_main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_smof_validate_header_valid);
    RUN_TEST(test_smof_validate_header_invalid_magic);
    RUN_TEST(test_smof_validate_header_invalid_version);
    RUN_TEST(test_smof_validate_header_null_header);
    RUN_TEST(test_smof_get_header_size);
    RUN_TEST(test_smof_header_alignment);
    RUN_TEST(test_smof_magic_constant);
    RUN_TEST(test_smof_section_type_values);
    RUN_TEST(test_smof_symbol_binding_values);
    RUN_TEST(test_smof_symbol_type_values);
    RUN_TEST(test_smof_symbol_utility_functions);
    RUN_TEST(test_smof_relocation_utility_functions);
    
    return UNITY_END();
}

int main(void) {
    return test_smof_main();
}
