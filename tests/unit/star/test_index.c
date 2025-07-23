/**
 * @file test_index.c
 * @brief Unit tests for STAR file indexing system
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * Comprehensive unit tests for file indexing, search functionality,
 * index maintenance, and performance optimization features.
 * Tests cover hash-based indexing, sorted access, and metadata indexing.
 */

#include "unity.h"
#include "index.h"
#include "memory.h"
#include "error.h"
#include <string.h>
#include <stdlib.h>

/* Test fixture data */
static file_index_t* test_index;
static memory_pool_t* test_memory_pool;
static error_context_t* test_error_ctx;

/**
 * @brief Setup function called before each test
 */
void setUp(void) {
    test_error_ctx = error_context_create();
    TEST_ASSERT_NOT_NULL(test_error_ctx);
    
    test_memory_pool = memory_pool_create(32768);
    TEST_ASSERT_NOT_NULL(test_memory_pool);
    
    test_index = file_index_create(test_memory_pool, test_error_ctx);
    TEST_ASSERT_NOT_NULL(test_index);
}

/**
 * @brief Teardown function called after each test
 */
void tearDown(void) {
    if (test_index != NULL) {
        file_index_destroy(test_index);
        test_index = NULL;
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
 * @brief Test file index creation and destruction
 */
void test_file_index_lifecycle(void) {
    file_index_t* index = file_index_create(test_memory_pool, test_error_ctx);
    TEST_ASSERT_NOT_NULL(index);
    
    /* Initial state */
    TEST_ASSERT_EQUAL_size_t(0, file_index_get_entry_count(index));
    TEST_ASSERT_TRUE(file_index_is_empty(index));
    
    file_index_destroy(index);
    
    /* Test with NULL parameters */
    index = file_index_create(NULL, test_error_ctx);
    TEST_ASSERT_NULL(index);
    
    index = file_index_create(test_memory_pool, NULL);
    TEST_ASSERT_NULL(index);
    
    /* Test destruction with NULL */
    file_index_destroy(NULL); /* Should not crash */
}

/**
 * @brief Test basic index entry addition
 */
void test_index_entry_addition(void) {
    /* Create test index entries */
    index_entry_t entry1 = {
        .filename = "main.smof",
        .file_size = 1024,
        .offset = 0,
        .compressed_size = 800,
        .crc32 = 0x12345678,
        .timestamp = 1640995200,  /* 2022-01-01 00:00:00 UTC */
        .flags = INDEX_FLAG_COMPRESSED
    };
    
    index_entry_t entry2 = {
        .filename = "library.smof",
        .file_size = 2048,
        .offset = 1024,
        .compressed_size = 2048,  /* Not compressed */
        .crc32 = 0x87654321,
        .timestamp = 1640995260,  /* 2022-01-01 00:01:00 UTC */
        .flags = 0
    };
    
    /* Add entries to index */
    bool result = file_index_add_entry(test_index, &entry1);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    result = file_index_add_entry(test_index, &entry2);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    /* Verify index state */
    TEST_ASSERT_EQUAL_size_t(2, file_index_get_entry_count(test_index));
    TEST_ASSERT_FALSE(file_index_is_empty(test_index));
}

/**
 * @brief Test index entry lookup by filename
 */
void test_index_lookup_by_filename(void) {
    /* Add test entries */
    const char* filenames[] = {"alpha.smof", "beta.smof", "gamma.smof"};
    const size_t count = sizeof(filenames) / sizeof(filenames[0]);
    
    for (size_t i = 0; i < count; i++) {
        index_entry_t entry = {
            .filename = filenames[i],
            .file_size = (uint32_t)(100 + i * 50),
            .offset = (uint32_t)(i * 200),
            .compressed_size = (uint32_t)(80 + i * 40),
            .crc32 = (uint32_t)(0x1000 + i),
            .timestamp = (uint64_t)(1640995200 + i * 60),
            .flags = (i % 2) ? INDEX_FLAG_COMPRESSED : 0
        };
        
        bool result = file_index_add_entry(test_index, &entry);
        TEST_ASSERT_TRUE(result);
    }
    
    /* Test successful lookups */
    const index_entry_t* found1 = file_index_find_by_filename(test_index, "alpha.smof");
    const index_entry_t* found2 = file_index_find_by_filename(test_index, "beta.smof");
    const index_entry_t* found3 = file_index_find_by_filename(test_index, "gamma.smof");
    
    TEST_ASSERT_NOT_NULL(found1);
    TEST_ASSERT_NOT_NULL(found2);
    TEST_ASSERT_NOT_NULL(found3);
    
    TEST_ASSERT_EQUAL_STRING("alpha.smof", found1->filename);
    TEST_ASSERT_EQUAL_STRING("beta.smof", found2->filename);
    TEST_ASSERT_EQUAL_STRING("gamma.smof", found3->filename);
    
    TEST_ASSERT_EQUAL_size_t(100, found1->file_size);
    TEST_ASSERT_EQUAL_size_t(150, found2->file_size);
    TEST_ASSERT_EQUAL_size_t(200, found3->file_size);
    
    /* Test failed lookup */
    const index_entry_t* not_found = file_index_find_by_filename(test_index, "nonexistent.smof");
    TEST_ASSERT_NULL(not_found);
    
    /* Test NULL filename */
    const index_entry_t* null_lookup = file_index_find_by_filename(test_index, NULL);
    TEST_ASSERT_NULL(null_lookup);
}

/**
 * @brief Test index entry lookup by offset
 */
void test_index_lookup_by_offset(void) {
    /* Add entries with specific offsets */
    index_entry_t entries[] = {
        { .filename = "file1.smof", .offset = 0, .file_size = 100 },
        { .filename = "file2.smof", .offset = 1000, .file_size = 200 },
        { .filename = "file3.smof", .offset = 2000, .file_size = 150 },
        { .filename = "file4.smof", .offset = 5000, .file_size = 300 }
    };
    
    for (size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {
        bool result = file_index_add_entry(test_index, &entries[i]);
        TEST_ASSERT_TRUE(result);
    }
    
    /* Test exact offset lookups */
    const index_entry_t* found1 = file_index_find_by_offset(test_index, 0);
    const index_entry_t* found2 = file_index_find_by_offset(test_index, 1000);
    const index_entry_t* found3 = file_index_find_by_offset(test_index, 2000);
    const index_entry_t* found4 = file_index_find_by_offset(test_index, 5000);
    
    TEST_ASSERT_NOT_NULL(found1);
    TEST_ASSERT_NOT_NULL(found2);
    TEST_ASSERT_NOT_NULL(found3);
    TEST_ASSERT_NOT_NULL(found4);
    
    TEST_ASSERT_EQUAL_STRING("file1.smof", found1->filename);
    TEST_ASSERT_EQUAL_STRING("file2.smof", found2->filename);
    TEST_ASSERT_EQUAL_STRING("file3.smof", found3->filename);
    TEST_ASSERT_EQUAL_STRING("file4.smof", found4->filename);
    
    /* Test offset that doesn't exist */
    const index_entry_t* not_found = file_index_find_by_offset(test_index, 3000);
    TEST_ASSERT_NULL(not_found);
}

/**
 * @brief Test index entry removal
 */
void test_index_entry_removal(void) {
    /* Add multiple entries */
    const char* filenames[] = {"remove1.smof", "keep1.smof", "remove2.smof", "keep2.smof"};
    const size_t count = sizeof(filenames) / sizeof(filenames[0]);
    
    for (size_t i = 0; i < count; i++) {
        index_entry_t entry = {
            .filename = filenames[i],
            .file_size = (uint32_t)(100 + i * 10),
            .offset = (uint32_t)(i * 200),
            .compressed_size = (uint32_t)(90 + i * 10),
            .crc32 = (uint32_t)(0x1000 + i),
            .timestamp = (uint64_t)(1640995200 + i * 30)
        };
        
        bool result = file_index_add_entry(test_index, &entry);
        TEST_ASSERT_TRUE(result);
    }
    
    TEST_ASSERT_EQUAL_size_t(count, file_index_get_entry_count(test_index));
    
    /* Remove specific entries */
    bool result = file_index_remove_entry(test_index, "remove1.smof");
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_size_t(count - 1, file_index_get_entry_count(test_index));
    
    result = file_index_remove_entry(test_index, "remove2.smof");
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_size_t(count - 2, file_index_get_entry_count(test_index));
    
    /* Verify removed entries are gone */
    const index_entry_t* removed1 = file_index_find_by_filename(test_index, "remove1.smof");
    const index_entry_t* removed2 = file_index_find_by_filename(test_index, "remove2.smof");
    TEST_ASSERT_NULL(removed1);
    TEST_ASSERT_NULL(removed2);
    
    /* Verify kept entries are still there */
    const index_entry_t* kept1 = file_index_find_by_filename(test_index, "keep1.smof");
    const index_entry_t* kept2 = file_index_find_by_filename(test_index, "keep2.smof");
    TEST_ASSERT_NOT_NULL(kept1);
    TEST_ASSERT_NOT_NULL(kept2);
    
    /* Test removing non-existent entry */
    result = file_index_remove_entry(test_index, "nonexistent.smof");
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Test index iteration functionality
 */
void test_index_iteration(void) {
    /* Add multiple entries */
    const char* filenames[] = {"file1.smof", "file2.smof", "file3.smof", "file4.smof", "file5.smof"};
    const size_t count = sizeof(filenames) / sizeof(filenames[0]);
    
    for (size_t i = 0; i < count; i++) {
        index_entry_t entry = {
            .filename = filenames[i],
            .file_size = (uint32_t)(100 + i * 50),
            .offset = (uint32_t)(i * 300),
            .compressed_size = (uint32_t)(80 + i * 40),
            .crc32 = (uint32_t)(0x2000 + i),
            .timestamp = (uint64_t)(1640995200 + i * 120)
        };
        
        bool result = file_index_add_entry(test_index, &entry);
        TEST_ASSERT_TRUE(result);
    }
    
    /* Iterate through entries */
    size_t iteration_count = 0;
    bool found_files[count];
    memset(found_files, 0, sizeof(found_files));
    
    index_iterator_t iter = file_index_begin(test_index);
    while (index_iterator_is_valid(&iter)) {
        const index_entry_t* entry = index_iterator_get(&iter);
        TEST_ASSERT_NOT_NULL(entry);
        
        /* Find which file this is */
        for (size_t i = 0; i < count; i++) {
            if (strcmp(entry->filename, filenames[i]) == 0) {
                found_files[i] = true;
                TEST_ASSERT_EQUAL_size_t(100 + i * 50, entry->file_size);
                break;
            }
        }
        
        iteration_count++;
        index_iterator_next(&iter);
    }
    
    /* Verify all files were found */
    TEST_ASSERT_EQUAL_size_t(count, iteration_count);
    for (size_t i = 0; i < count; i++) {
        TEST_ASSERT_TRUE_MESSAGE(found_files[i], "File not found during iteration");
    }
}

/**
 * @brief Test index sorting functionality
 */
void test_index_sorting(void) {
    /* Add entries in random order */
    index_entry_t entries[] = {
        { .filename = "zebra.smof", .file_size = 500, .timestamp = 1640995500 },
        { .filename = "alpha.smof", .file_size = 100, .timestamp = 1640995100 },
        { .filename = "beta.smof", .file_size = 200, .timestamp = 1640995200 },
        { .filename = "gamma.smof", .file_size = 300, .timestamp = 1640995300 }
    };
    
    for (size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {
        bool result = file_index_add_entry(test_index, &entries[i]);
        TEST_ASSERT_TRUE(result);
    }
    
    /* Sort by filename */
    bool result = file_index_sort(test_index, INDEX_SORT_BY_FILENAME, INDEX_SORT_ASCENDING);
    TEST_ASSERT_TRUE(result);
    
    /* Verify sorted order */
    const char* expected_filename_order[] = {"alpha.smof", "beta.smof", "gamma.smof", "zebra.smof"};
    
    index_iterator_t iter = file_index_begin(test_index);
    for (size_t i = 0; index_iterator_is_valid(&iter); i++) {
        const index_entry_t* entry = index_iterator_get(&iter);
        TEST_ASSERT_EQUAL_STRING(expected_filename_order[i], entry->filename);
        index_iterator_next(&iter);
    }
    
    /* Sort by file size (descending) */
    result = file_index_sort(test_index, INDEX_SORT_BY_SIZE, INDEX_SORT_DESCENDING);
    TEST_ASSERT_TRUE(result);
    
    /* Verify sorted order */
    uint32_t expected_size_order[] = {500, 300, 200, 100};
    
    iter = file_index_begin(test_index);
    for (size_t i = 0; index_iterator_is_valid(&iter); i++) {
        const index_entry_t* entry = index_iterator_get(&iter);
        TEST_ASSERT_EQUAL_size_t(expected_size_order[i], entry->file_size);
        index_iterator_next(&iter);
    }
    
    /* Sort by timestamp */
    result = file_index_sort(test_index, INDEX_SORT_BY_TIMESTAMP, INDEX_SORT_ASCENDING);
    TEST_ASSERT_TRUE(result);
    
    /* Verify sorted order */
    uint64_t expected_timestamp_order[] = {1640995100, 1640995200, 1640995300, 1640995500};
    
    iter = file_index_begin(test_index);
    for (size_t i = 0; index_iterator_is_valid(&iter); i++) {
        const index_entry_t* entry = index_iterator_get(&iter);
        TEST_ASSERT_EQUAL_UINT64(expected_timestamp_order[i], entry->timestamp);
        index_iterator_next(&iter);
    }
}

/**
 * @brief Test index search functionality
 */
void test_index_search(void) {
    /* Add entries with searchable content */
    index_entry_t entries[] = {
        { .filename = "main.smof", .file_size = 1000, .flags = INDEX_FLAG_EXECUTABLE },
        { .filename = "library.smof", .file_size = 2000, .flags = INDEX_FLAG_COMPRESSED },
        { .filename = "utility.smof", .file_size = 500, .flags = 0 },
        { .filename = "test_main.smof", .file_size = 800, .flags = INDEX_FLAG_EXECUTABLE },
        { .filename = "support.smof", .file_size = 300, .flags = INDEX_FLAG_COMPRESSED }
    };
    
    for (size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {
        bool result = file_index_add_entry(test_index, &entries[i]);
        TEST_ASSERT_TRUE(result);
    }
    
    /* Search by filename pattern */
    index_search_result_t* search_result = file_index_search(test_index, "main", INDEX_SEARCH_FILENAME);
    TEST_ASSERT_NOT_NULL(search_result);
    TEST_ASSERT_EQUAL_size_t(2, index_search_result_get_count(search_result));
    
    /* Verify search results contain both main files */
    bool found_main = false, found_test_main = false;
    for (size_t i = 0; i < index_search_result_get_count(search_result); i++) {
        const index_entry_t* entry = index_search_result_get_entry(search_result, i);
        if (strcmp(entry->filename, "main.smof") == 0) {
            found_main = true;
        } else if (strcmp(entry->filename, "test_main.smof") == 0) {
            found_test_main = true;
        }
    }
    
    TEST_ASSERT_TRUE(found_main);
    TEST_ASSERT_TRUE(found_test_main);
    
    index_search_result_destroy(search_result);
    
    /* Search by size range */
    search_result = file_index_search_by_size_range(test_index, 400, 1000);
    TEST_ASSERT_NOT_NULL(search_result);
    TEST_ASSERT_EQUAL_size_t(3, index_search_result_get_count(search_result));  /* utility, test_main, main */
    
    /* Verify all results are within size range */
    for (size_t i = 0; i < index_search_result_get_count(search_result); i++) {
        const index_entry_t* entry = index_search_result_get_entry(search_result, i);
        TEST_ASSERT_GREATER_OR_EQUAL(400, entry->file_size);
        TEST_ASSERT_LESS_OR_EQUAL(1000, entry->file_size);
    }
    
    index_search_result_destroy(search_result);
    
    /* Search by flags */
    search_result = file_index_search_by_flags(test_index, INDEX_FLAG_COMPRESSED, true);
    TEST_ASSERT_NOT_NULL(search_result);
    TEST_ASSERT_EQUAL_size_t(2, index_search_result_get_count(search_result));  /* library, support */
    
    /* Verify all results have compressed flag */
    for (size_t i = 0; i < index_search_result_get_count(search_result); i++) {
        const index_entry_t* entry = index_search_result_get_entry(search_result, i);
        TEST_ASSERT_TRUE((entry->flags & INDEX_FLAG_COMPRESSED) != 0);
    }
    
    index_search_result_destroy(search_result);
}

/**
 * @brief Test index serialization and deserialization
 */
void test_index_serialization(void) {
    /* Add test entries */
    index_entry_t entries[] = {
        { .filename = "file1.smof", .file_size = 100, .offset = 0, .compressed_size = 80, .crc32 = 0x11111111, .timestamp = 1640995200 },
        { .filename = "file2.smof", .file_size = 200, .offset = 150, .compressed_size = 160, .crc32 = 0x22222222, .timestamp = 1640995300 },
        { .filename = "file3.smof", .file_size = 300, .offset = 350, .compressed_size = 250, .crc32 = 0x33333333, .timestamp = 1640995400 }
    };
    
    for (size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {
        bool result = file_index_add_entry(test_index, &entries[i]);
        TEST_ASSERT_TRUE(result);
    }
    
    /* Serialize index */
    size_t serialized_size = file_index_get_serialized_size(test_index);
    TEST_ASSERT_GREATER_THAN(0, serialized_size);
    
    uint8_t* buffer = malloc(serialized_size);
    TEST_ASSERT_NOT_NULL(buffer);
    
    bool result = file_index_serialize(test_index, buffer, serialized_size);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    /* Deserialize to new index */
    file_index_t* new_index = file_index_deserialize(buffer, serialized_size, test_memory_pool, test_error_ctx);
    TEST_ASSERT_NOT_NULL(new_index);
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    /* Verify deserialized index */
    TEST_ASSERT_EQUAL_size_t(file_index_get_entry_count(test_index), file_index_get_entry_count(new_index));
    
    /* Check each entry */
    for (size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {
        const index_entry_t* original = file_index_find_by_filename(test_index, entries[i].filename);
        const index_entry_t* deserialized = file_index_find_by_filename(new_index, entries[i].filename);
        
        TEST_ASSERT_NOT_NULL(original);
        TEST_ASSERT_NOT_NULL(deserialized);
        
        TEST_ASSERT_EQUAL_STRING(original->filename, deserialized->filename);
        TEST_ASSERT_EQUAL_size_t(original->file_size, deserialized->file_size);
        TEST_ASSERT_EQUAL_size_t(original->offset, deserialized->offset);
        TEST_ASSERT_EQUAL_size_t(original->compressed_size, deserialized->compressed_size);
        TEST_ASSERT_EQUAL_HEX32(original->crc32, deserialized->crc32);
        TEST_ASSERT_EQUAL_UINT64(original->timestamp, deserialized->timestamp);
    }
    
    /* Cleanup */
    free(buffer);
    file_index_destroy(new_index);
}

/**
 * @brief Test index performance optimization
 */
void test_index_performance_optimization(void) {
    /* Add many entries to test performance */
    const size_t entry_count = 1000;
    
    for (size_t i = 0; i < entry_count; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "file_%04zu.smof", i);
        
        index_entry_t entry = {
            .filename = filename,
            .file_size = (uint32_t)(100 + i),
            .offset = (uint32_t)(i * 200),
            .compressed_size = (uint32_t)(80 + i),
            .crc32 = (uint32_t)(0x10000 + i),
            .timestamp = (uint64_t)(1640995200 + i * 10)
        };
        
        bool result = file_index_add_entry(test_index, &entry);
        TEST_ASSERT_TRUE(result);
    }
    
    TEST_ASSERT_EQUAL_size_t(entry_count, file_index_get_entry_count(test_index));
    
    /* Test hash table performance - lookups should be fast */
    const char* test_filenames[] = {"file_0001.smof", "file_0500.smof", "file_0999.smof"};
    
    for (size_t i = 0; i < sizeof(test_filenames) / sizeof(test_filenames[0]); i++) {
        const index_entry_t* found = file_index_find_by_filename(test_index, test_filenames[i]);
        TEST_ASSERT_NOT_NULL(found);
        TEST_ASSERT_EQUAL_STRING(test_filenames[i], found->filename);
    }
    
    /* Test index optimization */
    bool result = file_index_optimize(test_index);
    if (result) {
        /* Optimization succeeded - verify functionality still works */
        const index_entry_t* found = file_index_find_by_filename(test_index, "file_0500.smof");
        TEST_ASSERT_NOT_NULL(found);
        TEST_ASSERT_EQUAL_STRING("file_0500.smof", found->filename);
    }
    
    /* Test statistics */
    index_stats_t stats = file_index_get_statistics(test_index);
    TEST_ASSERT_EQUAL_size_t(entry_count, stats.total_entries);
    TEST_ASSERT_GREATER_THAN(0, stats.hash_table_size);
    TEST_ASSERT_GREATER_OR_EQUAL(0.0f, stats.load_factor);
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, stats.load_factor);
}

/**
 * @brief Test index with NULL parameters
 */
void test_index_null_parameters(void) {
    /* Test operations with NULL index */
    TEST_ASSERT_EQUAL_size_t(0, file_index_get_entry_count(NULL));
    TEST_ASSERT_TRUE(file_index_is_empty(NULL));
    TEST_ASSERT_NULL(file_index_find_by_filename(NULL, "test.smof"));
    TEST_ASSERT_NULL(file_index_find_by_offset(NULL, 0));
    
    /* Test with NULL entry */
    bool result = file_index_add_entry(test_index, NULL);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
    
    error_context_clear(test_error_ctx);
    
    /* Test with invalid entry (NULL filename) */
    index_entry_t invalid_entry = {
        .filename = NULL,
        .file_size = 100,
        .offset = 0
    };
    
    result = file_index_add_entry(test_index, &invalid_entry);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Main test runner function
 */
int main(void) {
    UNITY_BEGIN();
    
    /* Basic functionality tests */
    RUN_TEST(test_file_index_lifecycle);
    RUN_TEST(test_index_entry_addition);
    RUN_TEST(test_index_lookup_by_filename);
    RUN_TEST(test_index_lookup_by_offset);
    
    /* Index management tests */
    RUN_TEST(test_index_entry_removal);
    RUN_TEST(test_index_iteration);
    RUN_TEST(test_index_sorting);
    
    /* Advanced functionality tests */
    RUN_TEST(test_index_search);
    RUN_TEST(test_index_serialization);
    RUN_TEST(test_index_performance_optimization);
    
    /* Edge case tests */
    RUN_TEST(test_index_null_parameters);
    
    return UNITY_END();
}
