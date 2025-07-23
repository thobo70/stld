/**
 * @file test_archive.c
 * @brief Unit tests for STAR archive management
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * Comprehensive unit tests for archive creation, manipulation, and extraction.
 * Tests cover archive format handling, file addition/removal, metadata
 * management, and archive validation.
 */

#include "unity.h"
#include "archive.h"
#include "memory.h"
#include "error.h"
#include <string.h>
#include <stdio.h>

/* Test fixture data */
static archive_t* test_archive;
static error_context_t* test_error_ctx;
static memory_pool_t* test_memory_pool;

/**
 * @brief Setup function called before each test
 */
void setUp(void) {
    test_error_ctx = error_context_create();
    TEST_ASSERT_NOT_NULL(test_error_ctx);
    
    test_memory_pool = memory_pool_create(32768);
    TEST_ASSERT_NOT_NULL(test_memory_pool);
    
    test_archive = archive_create(test_memory_pool, test_error_ctx);
    TEST_ASSERT_NOT_NULL(test_archive);
}

/**
 * @brief Teardown function called after each test
 */
void tearDown(void) {
    if (test_archive != NULL) {
        archive_destroy(test_archive);
        test_archive = NULL;
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
 * @brief Test archive creation and destruction
 */
void test_archive_lifecycle(void) {
    archive_t* archive = archive_create(test_memory_pool, test_error_ctx);
    TEST_ASSERT_NOT_NULL(archive);
    
    /* Initial state */
    TEST_ASSERT_EQUAL_size_t(0, archive_get_file_count(archive));
    TEST_ASSERT_TRUE(archive_is_empty(archive));
    TEST_ASSERT_FALSE(archive_is_modified(archive));
    
    archive_destroy(archive);
    
    /* Test with NULL parameters */
    archive = archive_create(NULL, test_error_ctx);
    TEST_ASSERT_NULL(archive);
    
    archive = archive_create(test_memory_pool, NULL);
    TEST_ASSERT_NULL(archive);
    
    /* Test destruction with NULL */
    archive_destroy(NULL); /* Should not crash */
}

/**
 * @brief Test adding files to archive
 */
void test_archive_add_file(void) {
    /* Create test file data */
    const char* filename1 = "test1.smof";
    const char* filename2 = "test2.smof";
    
    uint8_t file1_data[] = {0x53, 0x4D, 0x4F, 0x46, 0x01, 0x00};  /* SMOF header start */
    uint8_t file2_data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x21};  /* "Hello!" */
    
    /* Add files to archive */
    archive_file_id_t id1 = archive_add_file(test_archive, filename1, file1_data, sizeof(file1_data));
    archive_file_id_t id2 = archive_add_file(test_archive, filename2, file2_data, sizeof(file2_data));
    
    TEST_ASSERT_NOT_EQUAL(ARCHIVE_FILE_ID_INVALID, id1);
    TEST_ASSERT_NOT_EQUAL(ARCHIVE_FILE_ID_INVALID, id2);
    TEST_ASSERT_NOT_EQUAL(id1, id2);
    
    /* Verify archive state */
    TEST_ASSERT_EQUAL_size_t(2, archive_get_file_count(test_archive));
    TEST_ASSERT_FALSE(archive_is_empty(test_archive));
    TEST_ASSERT_TRUE(archive_is_modified(test_archive));
    
    /* Verify no errors occurred */
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Test file lookup in archive
 */
void test_archive_file_lookup(void) {
    /* Add test files */
    const char* filenames[] = {"main.smof", "lib.smof", "util.smof"};
    uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04};
    
    archive_file_id_t file_ids[3];
    for (size_t i = 0; i < 3; i++) {
        file_ids[i] = archive_add_file(test_archive, filenames[i], test_data, sizeof(test_data));
        TEST_ASSERT_NOT_EQUAL(ARCHIVE_FILE_ID_INVALID, file_ids[i]);
    }
    
    /* Test successful lookups */
    archive_file_id_t found1 = archive_find_file(test_archive, "main.smof");
    archive_file_id_t found2 = archive_find_file(test_archive, "lib.smof");
    archive_file_id_t found3 = archive_find_file(test_archive, "util.smof");
    
    TEST_ASSERT_EQUAL(file_ids[0], found1);
    TEST_ASSERT_EQUAL(file_ids[1], found2);
    TEST_ASSERT_EQUAL(file_ids[2], found3);
    
    /* Test failed lookup */
    archive_file_id_t not_found = archive_find_file(test_archive, "nonexistent.smof");
    TEST_ASSERT_EQUAL(ARCHIVE_FILE_ID_INVALID, not_found);
    
    /* Test NULL filename */
    archive_file_id_t null_lookup = archive_find_file(test_archive, NULL);
    TEST_ASSERT_EQUAL(ARCHIVE_FILE_ID_INVALID, null_lookup);
}

/**
 * @brief Test file retrieval from archive
 */
void test_archive_file_retrieval(void) {
    /* Add test file */
    const char* filename = "test.smof";
    uint8_t original_data[] = {
        0x53, 0x4D, 0x4F, 0x46,  /* SMOF magic */
        0x01, 0x00,              /* Version */
        0x12, 0x34, 0x56, 0x78   /* Test data */
    };
    
    archive_file_id_t file_id = archive_add_file(test_archive, filename, original_data, sizeof(original_data));
    TEST_ASSERT_NOT_EQUAL(ARCHIVE_FILE_ID_INVALID, file_id);
    
    /* Retrieve file info */
    const archive_file_info_t* file_info = archive_get_file_info(test_archive, file_id);
    TEST_ASSERT_NOT_NULL(file_info);
    
    /* Verify file info */
    TEST_ASSERT_EQUAL_STRING(filename, file_info->name);
    TEST_ASSERT_EQUAL_size_t(sizeof(original_data), file_info->size);
    TEST_ASSERT_GREATER_THAN(0, file_info->timestamp);
    TEST_ASSERT_EQUAL_HEX32(archive_calculate_crc32(original_data, sizeof(original_data)), file_info->crc32);
    
    /* Retrieve file data */
    size_t retrieved_size;
    const uint8_t* retrieved_data = archive_get_file_data(test_archive, file_id, &retrieved_size);
    TEST_ASSERT_NOT_NULL(retrieved_data);
    TEST_ASSERT_EQUAL_size_t(sizeof(original_data), retrieved_size);
    TEST_ASSERT_EQUAL_MEMORY(original_data, retrieved_data, sizeof(original_data));
    
    /* Test invalid file ID */
    const archive_file_info_t* invalid_info = archive_get_file_info(test_archive, ARCHIVE_FILE_ID_INVALID);
    TEST_ASSERT_NULL(invalid_info);
    
    const uint8_t* invalid_data = archive_get_file_data(test_archive, ARCHIVE_FILE_ID_INVALID, &retrieved_size);
    TEST_ASSERT_NULL(invalid_data);
}

/**
 * @brief Test file removal from archive
 */
void test_archive_file_removal(void) {
    /* Add multiple files */
    const char* filenames[] = {"file1.smof", "file2.smof", "file3.smof"};
    uint8_t test_data[] = {0xAA, 0xBB, 0xCC, 0xDD};
    
    archive_file_id_t file_ids[3];
    for (size_t i = 0; i < 3; i++) {
        file_ids[i] = archive_add_file(test_archive, filenames[i], test_data, sizeof(test_data));
        TEST_ASSERT_NOT_EQUAL(ARCHIVE_FILE_ID_INVALID, file_ids[i]);
    }
    
    TEST_ASSERT_EQUAL_size_t(3, archive_get_file_count(test_archive));
    
    /* Remove middle file */
    bool result = archive_remove_file(test_archive, file_ids[1]);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_size_t(2, archive_get_file_count(test_archive));
    
    /* Verify removed file is no longer accessible */
    const archive_file_info_t* removed_info = archive_get_file_info(test_archive, file_ids[1]);
    TEST_ASSERT_NULL(removed_info);
    
    archive_file_id_t removed_lookup = archive_find_file(test_archive, filenames[1]);
    TEST_ASSERT_EQUAL(ARCHIVE_FILE_ID_INVALID, removed_lookup);
    
    /* Verify other files are still accessible */
    const archive_file_info_t* info1 = archive_get_file_info(test_archive, file_ids[0]);
    const archive_file_info_t* info3 = archive_get_file_info(test_archive, file_ids[2]);
    TEST_ASSERT_NOT_NULL(info1);
    TEST_ASSERT_NOT_NULL(info3);
    TEST_ASSERT_EQUAL_STRING(filenames[0], info1->name);
    TEST_ASSERT_EQUAL_STRING(filenames[2], info3->name);
    
    /* Test removing invalid file ID */
    result = archive_remove_file(test_archive, ARCHIVE_FILE_ID_INVALID);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Test archive file iteration
 */
void test_archive_file_iteration(void) {
    /* Add multiple files */
    const char* filenames[] = {"alpha.smof", "beta.smof", "gamma.smof", "delta.smof"};
    const size_t file_count = sizeof(filenames) / sizeof(filenames[0]);
    
    uint8_t test_data[] = {0x12, 0x34, 0x56, 0x78};
    
    for (size_t i = 0; i < file_count; i++) {
        archive_file_id_t id = archive_add_file(test_archive, filenames[i], test_data, sizeof(test_data));
        TEST_ASSERT_NOT_EQUAL(ARCHIVE_FILE_ID_INVALID, id);
    }
    
    /* Iterate through files */
    size_t iteration_count = 0;
    bool found_files[file_count];
    memset(found_files, 0, sizeof(found_files));
    
    archive_iterator_t iter = archive_begin(test_archive);
    while (archive_iterator_is_valid(&iter)) {
        const archive_file_info_t* file_info = archive_iterator_get(&iter);
        TEST_ASSERT_NOT_NULL(file_info);
        
        /* Find which file this is */
        for (size_t i = 0; i < file_count; i++) {
            if (strcmp(file_info->name, filenames[i]) == 0) {
                found_files[i] = true;
                TEST_ASSERT_EQUAL_size_t(sizeof(test_data), file_info->size);
                break;
            }
        }
        
        iteration_count++;
        archive_iterator_next(&iter);
    }
    
    /* Verify all files were found */
    TEST_ASSERT_EQUAL_size_t(file_count, iteration_count);
    for (size_t i = 0; i < file_count; i++) {
        TEST_ASSERT_TRUE_MESSAGE(found_files[i], "File not found during iteration");
    }
}

/**
 * @brief Test archive serialization to buffer
 */
void test_archive_serialization(void) {
    /* Add test files */
    const char* file1_name = "program.smof";
    const char* file2_name = "library.smof";
    
    uint8_t file1_data[] = {0x53, 0x4D, 0x4F, 0x46, 0x01, 0x00, 0x00, 0x00};
    uint8_t file2_data[] = {0x4C, 0x49, 0x42, 0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};
    
    archive_add_file(test_archive, file1_name, file1_data, sizeof(file1_data));
    archive_add_file(test_archive, file2_name, file2_data, sizeof(file2_data));
    
    /* Calculate required buffer size */
    size_t buffer_size = archive_get_serialized_size(test_archive);
    TEST_ASSERT_GREATER_THAN(sizeof(file1_data) + sizeof(file2_data), buffer_size);
    
    /* Serialize to buffer */
    uint8_t* buffer = malloc(buffer_size);
    TEST_ASSERT_NOT_NULL(buffer);
    
    bool result = archive_serialize(test_archive, buffer, buffer_size);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    /* Deserialize to new archive */
    archive_t* new_archive = archive_deserialize(buffer, buffer_size, test_memory_pool, test_error_ctx);
    TEST_ASSERT_NOT_NULL(new_archive);
    
    /* Verify deserialized archive */
    TEST_ASSERT_EQUAL_size_t(archive_get_file_count(test_archive), archive_get_file_count(new_archive));
    
    /* Check first file */
    archive_file_id_t id1 = archive_find_file(new_archive, file1_name);
    TEST_ASSERT_NOT_EQUAL(ARCHIVE_FILE_ID_INVALID, id1);
    
    size_t size1;
    const uint8_t* data1 = archive_get_file_data(new_archive, id1, &size1);
    TEST_ASSERT_NOT_NULL(data1);
    TEST_ASSERT_EQUAL_size_t(sizeof(file1_data), size1);
    TEST_ASSERT_EQUAL_MEMORY(file1_data, data1, sizeof(file1_data));
    
    /* Check second file */
    archive_file_id_t id2 = archive_find_file(new_archive, file2_name);
    TEST_ASSERT_NOT_EQUAL(ARCHIVE_FILE_ID_INVALID, id2);
    
    size_t size2;
    const uint8_t* data2 = archive_get_file_data(new_archive, id2, &size2);
    TEST_ASSERT_NOT_NULL(data2);
    TEST_ASSERT_EQUAL_size_t(sizeof(file2_data), size2);
    TEST_ASSERT_EQUAL_MEMORY(file2_data, data2, sizeof(file2_data));
    
    /* Cleanup */
    free(buffer);
    archive_destroy(new_archive);
}

/**
 * @brief Test archive file writing and reading
 */
void test_archive_file_io(void) {
    /* Add test files */
    const char* files[] = {"test1.smof", "test2.smof"};
    uint8_t data1[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t data2[] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    
    archive_add_file(test_archive, files[0], data1, sizeof(data1));
    archive_add_file(test_archive, files[1], data2, sizeof(data2));
    
    /* Write archive to file */
    const char* archive_filename = "/tmp/test_archive.star";
    bool result = archive_write_to_file(test_archive, archive_filename);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    /* Read archive from file */
    archive_t* loaded_archive = archive_read_from_file(archive_filename, test_memory_pool, test_error_ctx);
    TEST_ASSERT_NOT_NULL(loaded_archive);
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    /* Verify loaded archive */
    TEST_ASSERT_EQUAL_size_t(2, archive_get_file_count(loaded_archive));
    
    /* Check files */
    archive_file_id_t id1 = archive_find_file(loaded_archive, files[0]);
    archive_file_id_t id2 = archive_find_file(loaded_archive, files[1]);
    
    TEST_ASSERT_NOT_EQUAL(ARCHIVE_FILE_ID_INVALID, id1);
    TEST_ASSERT_NOT_EQUAL(ARCHIVE_FILE_ID_INVALID, id2);
    
    size_t size1, size2;
    const uint8_t* loaded_data1 = archive_get_file_data(loaded_archive, id1, &size1);
    const uint8_t* loaded_data2 = archive_get_file_data(loaded_archive, id2, &size2);
    
    TEST_ASSERT_EQUAL_size_t(sizeof(data1), size1);
    TEST_ASSERT_EQUAL_size_t(sizeof(data2), size2);
    TEST_ASSERT_EQUAL_MEMORY(data1, loaded_data1, sizeof(data1));
    TEST_ASSERT_EQUAL_MEMORY(data2, loaded_data2, sizeof(data2));
    
    /* Cleanup */
    archive_destroy(loaded_archive);
    remove(archive_filename);
}

/**
 * @brief Test archive metadata handling
 */
void test_archive_metadata(void) {
    /* Set archive metadata */
    const char* creator = "STAR Unit Test";
    const char* comment = "Test archive for unit testing";
    
    bool result = archive_set_creator(test_archive, creator);
    TEST_ASSERT_TRUE(result);
    
    result = archive_set_comment(test_archive, comment);
    TEST_ASSERT_TRUE(result);
    
    /* Retrieve and verify metadata */
    const char* retrieved_creator = archive_get_creator(test_archive);
    const char* retrieved_comment = archive_get_comment(test_archive);
    
    TEST_ASSERT_NOT_NULL(retrieved_creator);
    TEST_ASSERT_NOT_NULL(retrieved_comment);
    TEST_ASSERT_EQUAL_STRING(creator, retrieved_creator);
    TEST_ASSERT_EQUAL_STRING(comment, retrieved_comment);
    
    /* Test timestamp */
    time_t creation_time = archive_get_creation_time(test_archive);
    TEST_ASSERT_GREATER_THAN(0, creation_time);
    
    time_t current_time = time(NULL);
    TEST_ASSERT_LESS_THAN(60, current_time - creation_time); /* Created within last minute */
    
    /* Test version */
    uint16_t version = archive_get_format_version(test_archive);
    TEST_ASSERT_EQUAL_INT(STAR_FORMAT_VERSION, version);
}

/**
 * @brief Test archive validation
 */
void test_archive_validation(void) {
    /* Add valid SMOF file */
    const char* valid_filename = "valid.smof";
    uint8_t valid_smof[] = {
        0x53, 0x4D, 0x4F, 0x46,  /* SMOF magic */
        0x01, 0x00,              /* Version 1 */
        0x00, 0x00,              /* Flags */
        /* ... rest of valid SMOF header ... */
    };
    
    archive_file_id_t valid_id = archive_add_file(test_archive, valid_filename, valid_smof, sizeof(valid_smof));
    TEST_ASSERT_NOT_EQUAL(ARCHIVE_FILE_ID_INVALID, valid_id);
    
    /* Add invalid file (wrong extension) */
    const char* invalid_filename = "invalid.txt";
    uint8_t invalid_data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};  /* "Hello" */
    
    archive_file_id_t invalid_id = archive_add_file(test_archive, invalid_filename, invalid_data, sizeof(invalid_data));
    
    /* Should succeed (validation might be optional) or fail gracefully */
    if (invalid_id == ARCHIVE_FILE_ID_INVALID) {
        TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
    }
    
    /* Validate archive */
    archive_validation_result_t validation = archive_validate(test_archive);
    
    /* Should have at least one valid file */
    TEST_ASSERT_GREATER_THAN(0, validation.valid_file_count);
    TEST_ASSERT_EQUAL_size_t(archive_get_file_count(test_archive), 
                            validation.valid_file_count + validation.invalid_file_count);
}

/**
 * @brief Test archive compression (if supported)
 */
void test_archive_compression(void) {
    /* Add compressible file (repeated data) */
    const char* filename = "compressible.smof";
    uint8_t compressible_data[1024];
    memset(compressible_data, 0xAA, sizeof(compressible_data));  /* Highly compressible */
    
    archive_file_id_t file_id = archive_add_file(test_archive, filename, compressible_data, sizeof(compressible_data));
    TEST_ASSERT_NOT_EQUAL(ARCHIVE_FILE_ID_INVALID, file_id);
    
    /* Enable compression */
    bool result = archive_set_compression(test_archive, ARCHIVE_COMPRESSION_GZIP);
    if (result) {
        /* Compression is supported */
        
        /* Serialize compressed archive */
        size_t compressed_size = archive_get_serialized_size(test_archive);
        
        /* Disable compression and compare */
        result = archive_set_compression(test_archive, ARCHIVE_COMPRESSION_NONE);
        TEST_ASSERT_TRUE(result);
        
        size_t uncompressed_size = archive_get_serialized_size(test_archive);
        
        /* Re-enable compression */
        result = archive_set_compression(test_archive, ARCHIVE_COMPRESSION_GZIP);
        TEST_ASSERT_TRUE(result);
        
        /* Compressed size should be significantly smaller */
        TEST_ASSERT_LESS_THAN(uncompressed_size / 2, compressed_size);
    } else {
        /* Compression not supported - that's okay */
        TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
    }
}

/**
 * @brief Test archive with NULL parameters
 */
void test_archive_null_parameters(void) {
    /* Test operations with NULL archive */
    TEST_ASSERT_EQUAL_size_t(0, archive_get_file_count(NULL));
    TEST_ASSERT_TRUE(archive_is_empty(NULL));
    TEST_ASSERT_EQUAL(ARCHIVE_FILE_ID_INVALID, archive_find_file(NULL, "test.smof"));
    TEST_ASSERT_NULL(archive_get_file_info(NULL, 0));
    
    /* Test with NULL filename */
    uint8_t test_data[] = {0x01, 0x02, 0x03};
    archive_file_id_t id = archive_add_file(test_archive, NULL, test_data, sizeof(test_data));
    TEST_ASSERT_EQUAL(ARCHIVE_FILE_ID_INVALID, id);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
    
    error_context_clear(test_error_ctx);
    
    /* Test with NULL data */
    id = archive_add_file(test_archive, "test.smof", NULL, 100);
    TEST_ASSERT_EQUAL(ARCHIVE_FILE_ID_INVALID, id);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Main test runner function
 */
int main(void) {
    UNITY_BEGIN();
    
    /* Basic functionality tests */
    RUN_TEST(test_archive_lifecycle);
    RUN_TEST(test_archive_add_file);
    RUN_TEST(test_archive_file_lookup);
    RUN_TEST(test_archive_file_retrieval);
    
    /* File management tests */
    RUN_TEST(test_archive_file_removal);
    RUN_TEST(test_archive_file_iteration);
    
    /* Serialization tests */
    RUN_TEST(test_archive_serialization);
    RUN_TEST(test_archive_file_io);
    
    /* Advanced functionality tests */
    RUN_TEST(test_archive_metadata);
    RUN_TEST(test_archive_validation);
    RUN_TEST(test_archive_compression);
    
    /* Edge case tests */
    RUN_TEST(test_archive_null_parameters);
    
    return UNITY_END();
}
