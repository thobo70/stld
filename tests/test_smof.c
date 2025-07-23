/* tests/test_smof.c */
#include "unity.h"
#include "smof.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>

/**
 * @file test_smof.c
 * @brief Unit tests for SMOF format validation
 * @details Tests SMOF header validation, section parsing, and format compliance
 */

/* Test data buffers */
static uint8_t valid_smof_header[] = {
    'S', 'M', 'O', 'F',  /* magic */
    0x01, 0x00,          /* version */
    0x01,                /* flags */
    0x00,                /* reserved */
    0x20, 0x00, 0x00, 0x00,  /* header_size */
    0x00, 0x01, 0x00, 0x00,  /* section_count */
    0x00, 0x00, 0x00, 0x00,  /* symbol_count */
    0x00, 0x00, 0x00, 0x00,  /* relocation_count */
    0x00, 0x00, 0x00, 0x00,  /* reserved2 */
    0x00, 0x00, 0x00, 0x00,  /* reserved3 */
    0x00, 0x00, 0x00, 0x00,  /* reserved4 */
    0x00, 0x00, 0x00, 0x00   /* reserved5 */
};

static uint8_t invalid_magic[] = {
    'B', 'A', 'D', '!',  /* bad magic */
    0x01, 0x00,          /* version */
    0x01,                /* flags */
    0x00,                /* reserved */
    0x20, 0x00, 0x00, 0x00,  /* header_size */
    0x00, 0x01, 0x00, 0x00,  /* section_count */
    0x00, 0x00, 0x00, 0x00,  /* symbol_count */
    0x00, 0x00, 0x00, 0x00,  /* relocation_count */
    0x00, 0x00, 0x00, 0x00,  /* reserved2 */
    0x00, 0x00, 0x00, 0x00,  /* reserved3 */
    0x00, 0x00, 0x00, 0x00,  /* reserved4 */
    0x00, 0x00, 0x00, 0x00   /* reserved5 */
};

void setUp(void) {
    /* Setup before each test */
}

void tearDown(void) {
    /* Cleanup after each test */
}

void test_smof_validate_header_valid(void) {
    smof_header_t header;
    smof_error_t result = smof_parse_header(valid_smof_header, sizeof(valid_smof_header), &header);
    
    TEST_ASSERT_EQUAL_INT(SMOF_SUCCESS, result);
    TEST_ASSERT_EQUAL_UINT32(SMOF_MAGIC, header.magic);
    TEST_ASSERT_EQUAL_UINT16(1, header.version);
    TEST_ASSERT_EQUAL_UINT8(1, header.flags);
    TEST_ASSERT_EQUAL_UINT32(32, header.header_size);
    TEST_ASSERT_EQUAL_UINT32(1, header.section_count);
}

void test_smof_validate_header_invalid_magic(void) {
    smof_header_t header;
    smof_error_t result = smof_parse_header(invalid_magic, sizeof(invalid_magic), &header);
    
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_INVALID_MAGIC, result);
}

void test_smof_validate_header_null_data(void) {
    smof_header_t header;
    smof_error_t result = smof_parse_header(NULL, sizeof(valid_smof_header), &header);
    
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_INVALID_ARGUMENT, result);
}

void test_smof_validate_header_null_header(void) {
    smof_error_t result = smof_parse_header(valid_smof_header, sizeof(valid_smof_header), NULL);
    
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_INVALID_ARGUMENT, result);
}

void test_smof_validate_header_insufficient_size(void) {
    smof_header_t header;
    smof_error_t result = smof_parse_header(valid_smof_header, 16, &header);
    
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_INSUFFICIENT_DATA, result);
}

void test_smof_validate_header_version_check(void) {
    uint8_t future_version[32];
    memcpy(future_version, valid_smof_header, sizeof(valid_smof_header));
    
    /* Set version to 999 */
    future_version[4] = 0xE7;  /* 999 & 0xFF */
    future_version[5] = 0x03;  /* (999 >> 8) & 0xFF */
    
    smof_header_t header;
    smof_error_t result = smof_parse_header(future_version, sizeof(future_version), &header);
    
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_UNSUPPORTED_VERSION, result);
}

void test_smof_validate_file_valid(void) {
    smof_error_t result = smof_validate_file(valid_smof_header, sizeof(valid_smof_header));
    
    /* Should succeed even with minimal data since we have valid header */
    TEST_ASSERT_EQUAL_INT(SMOF_SUCCESS, result);
}

void test_smof_validate_file_invalid(void) {
    smof_error_t result = smof_validate_file(invalid_magic, sizeof(invalid_magic));
    
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_INVALID_MAGIC, result);
}

void test_smof_validate_file_null_data(void) {
    smof_error_t result = smof_validate_file(NULL, 100);
    
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_INVALID_ARGUMENT, result);
}

void test_smof_validate_file_zero_size(void) {
    smof_error_t result = smof_validate_file(valid_smof_header, 0);
    
    TEST_ASSERT_EQUAL_INT(SMOF_ERROR_INSUFFICIENT_DATA, result);
}

void test_smof_get_header_size(void) {
    size_t size = smof_get_header_size();
    
    TEST_ASSERT_EQUAL_UINT(sizeof(smof_header_t), size);
    TEST_ASSERT_EQUAL_UINT(32, size);
}

void test_smof_header_alignment(void) {
    /* Verify header structure alignment */
    TEST_ASSERT_EQUAL_UINT(0, offsetof(smof_header_t, magic));
    TEST_ASSERT_EQUAL_UINT(4, offsetof(smof_header_t, version));
    TEST_ASSERT_EQUAL_UINT(6, offsetof(smof_header_t, flags));
    TEST_ASSERT_EQUAL_UINT(7, offsetof(smof_header_t, reserved));
    TEST_ASSERT_EQUAL_UINT(8, offsetof(smof_header_t, header_size));
}

void test_smof_magic_constant(void) {
    /* Verify magic constant value */
    const uint32_t expected_magic = ('S' << 0) | ('M' << 8) | ('O' << 16) | ('F' << 24);
    TEST_ASSERT_EQUAL_HEX32(expected_magic, SMOF_MAGIC);
}

void test_smof_section_type_values(void) {
    /* Verify section type constants */
    TEST_ASSERT_EQUAL_UINT8(0, SMOF_SECTION_NULL);
    TEST_ASSERT_EQUAL_UINT8(1, SMOF_SECTION_TEXT);
    TEST_ASSERT_EQUAL_UINT8(2, SMOF_SECTION_DATA);
    TEST_ASSERT_EQUAL_UINT8(3, SMOF_SECTION_BSS);
    TEST_ASSERT_EQUAL_UINT8(4, SMOF_SECTION_SYMTAB);
    TEST_ASSERT_EQUAL_UINT8(5, SMOF_SECTION_STRTAB);
    TEST_ASSERT_EQUAL_UINT8(6, SMOF_SECTION_RELTAB);
}

void test_smof_symbol_binding_values(void) {
    /* Verify symbol binding constants */
    TEST_ASSERT_EQUAL_UINT8(0, SMOF_SYMBOL_BIND_LOCAL);
    TEST_ASSERT_EQUAL_UINT8(1, SMOF_SYMBOL_BIND_GLOBAL);
    TEST_ASSERT_EQUAL_UINT8(2, SMOF_SYMBOL_BIND_WEAK);
}

void test_smof_symbol_type_values(void) {
    /* Verify symbol type constants */
    TEST_ASSERT_EQUAL_UINT8(0, SMOF_SYMBOL_TYPE_NOTYPE);
    TEST_ASSERT_EQUAL_UINT8(1, SMOF_SYMBOL_TYPE_OBJECT);
    TEST_ASSERT_EQUAL_UINT8(2, SMOF_SYMBOL_TYPE_FUNC);
    TEST_ASSERT_EQUAL_UINT8(3, SMOF_SYMBOL_TYPE_SECTION);
}

int test_smof_main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_smof_validate_header_valid);
    RUN_TEST(test_smof_validate_header_invalid_magic);
    RUN_TEST(test_smof_validate_header_null_data);
    RUN_TEST(test_smof_validate_header_null_header);
    RUN_TEST(test_smof_validate_header_insufficient_size);
    RUN_TEST(test_smof_validate_header_version_check);
    RUN_TEST(test_smof_validate_file_valid);
    RUN_TEST(test_smof_validate_file_invalid);
    RUN_TEST(test_smof_validate_file_null_data);
    RUN_TEST(test_smof_validate_file_zero_size);
    RUN_TEST(test_smof_get_header_size);
    RUN_TEST(test_smof_header_alignment);
    RUN_TEST(test_smof_magic_constant);
    RUN_TEST(test_smof_section_type_values);
    RUN_TEST(test_smof_symbol_binding_values);
    RUN_TEST(test_smof_symbol_type_values);
    
    return UNITY_END();
}

int main(void) {
    return test_smof_main();
}
