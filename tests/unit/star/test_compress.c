/**
 * @file test_compress.c
 * @brief Unit tests for STAR compression engine
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * Comprehensive unit tests for compression and decompression functionality.
 * Tests cover different compression algorithms, compression ratios,
 * data integrity, and error handling.
 */

#include "unity.h"
#include "compress.h"
#include "memory.h"
#include "error.h"
#include <string.h>
#include <stdlib.h>

/* Test fixture data */
static compression_engine_t* test_engine;
static memory_pool_t* test_memory_pool;
static error_context_t* test_error_ctx;

/**
 * @brief Setup function called before each test
 */
void setUp(void) {
    test_error_ctx = error_context_create();
    TEST_ASSERT_NOT_NULL(test_error_ctx);
    
    test_memory_pool = memory_pool_create(65536);
    TEST_ASSERT_NOT_NULL(test_memory_pool);
    
    test_engine = compression_engine_create(test_memory_pool, test_error_ctx);
    TEST_ASSERT_NOT_NULL(test_engine);
}

/**
 * @brief Teardown function called after each test
 */
void tearDown(void) {
    if (test_engine != NULL) {
        compression_engine_destroy(test_engine);
        test_engine = NULL;
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
 * @brief Test compression engine creation and destruction
 */
void test_compression_engine_lifecycle(void) {
    compression_engine_t* engine = compression_engine_create(test_memory_pool, test_error_ctx);
    TEST_ASSERT_NOT_NULL(engine);
    
    /* Verify initial state */
    compression_algorithm_t default_algo = compression_engine_get_algorithm(engine);
    TEST_ASSERT_TRUE(compression_algorithm_is_valid(default_algo));
    
    compression_level_t default_level = compression_engine_get_level(engine);
    TEST_ASSERT_TRUE(compression_level_is_valid(default_level));
    
    compression_engine_destroy(engine);
    
    /* Test with NULL parameters */
    engine = compression_engine_create(NULL, test_error_ctx);
    TEST_ASSERT_NULL(engine);
    
    engine = compression_engine_create(test_memory_pool, NULL);
    TEST_ASSERT_NULL(engine);
    
    /* Test destruction with NULL */
    compression_engine_destroy(NULL); /* Should not crash */
}

/**
 * @brief Test compression algorithm configuration
 */
void test_compression_algorithm_configuration(void) {
    /* Test setting different algorithms */
    bool result = compression_engine_set_algorithm(test_engine, COMPRESSION_ALGORITHM_NONE);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(COMPRESSION_ALGORITHM_NONE, compression_engine_get_algorithm(test_engine));
    
    result = compression_engine_set_algorithm(test_engine, COMPRESSION_ALGORITHM_DEFLATE);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(COMPRESSION_ALGORITHM_DEFLATE, compression_engine_get_algorithm(test_engine));
    
    result = compression_engine_set_algorithm(test_engine, COMPRESSION_ALGORITHM_GZIP);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(COMPRESSION_ALGORITHM_GZIP, compression_engine_get_algorithm(test_engine));
    
    result = compression_engine_set_algorithm(test_engine, COMPRESSION_ALGORITHM_LZ4);
    if (result) {
        /* LZ4 is supported */
        TEST_ASSERT_EQUAL_INT(COMPRESSION_ALGORITHM_LZ4, compression_engine_get_algorithm(test_engine));
    } else {
        /* LZ4 not supported - should have error */
        TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
        error_context_clear(test_error_ctx);
    }
    
    /* Test invalid algorithm */
    result = compression_engine_set_algorithm(test_engine, (compression_algorithm_t)999);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Test compression level configuration
 */
void test_compression_level_configuration(void) {
    /* Test setting different compression levels */
    bool result = compression_engine_set_level(test_engine, COMPRESSION_LEVEL_FASTEST);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(COMPRESSION_LEVEL_FASTEST, compression_engine_get_level(test_engine));
    
    result = compression_engine_set_level(test_engine, COMPRESSION_LEVEL_FAST);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(COMPRESSION_LEVEL_FAST, compression_engine_get_level(test_engine));
    
    result = compression_engine_set_level(test_engine, COMPRESSION_LEVEL_BALANCED);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(COMPRESSION_LEVEL_BALANCED, compression_engine_get_level(test_engine));
    
    result = compression_engine_set_level(test_engine, COMPRESSION_LEVEL_BEST);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(COMPRESSION_LEVEL_BEST, compression_engine_get_level(test_engine));
    
    /* Test invalid level */
    result = compression_engine_set_level(test_engine, (compression_level_t)999);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Test no compression (pass-through)
 */
void test_no_compression(void) {
    /* Configure for no compression */
    bool result = compression_engine_set_algorithm(test_engine, COMPRESSION_ALGORITHM_NONE);
    TEST_ASSERT_TRUE(result);
    
    /* Test data */
    const char* test_string = "Hello, World! This is a test string for compression.";
    size_t input_size = strlen(test_string);
    
    /* Compress data */
    size_t compressed_size;
    uint8_t* compressed_data = compression_engine_compress(test_engine, 
                                                          (const uint8_t*)test_string, 
                                                          input_size, 
                                                          &compressed_size);
    
    TEST_ASSERT_NOT_NULL(compressed_data);
    TEST_ASSERT_EQUAL_size_t(input_size, compressed_size);  /* No compression */
    TEST_ASSERT_EQUAL_MEMORY(test_string, compressed_data, input_size);
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    /* Decompress data */
    size_t decompressed_size;
    uint8_t* decompressed_data = compression_engine_decompress(test_engine,
                                                              compressed_data,
                                                              compressed_size,
                                                              &decompressed_size);
    
    TEST_ASSERT_NOT_NULL(decompressed_data);
    TEST_ASSERT_EQUAL_size_t(input_size, decompressed_size);
    TEST_ASSERT_EQUAL_MEMORY(test_string, decompressed_data, input_size);
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    /* Cleanup */
    compression_engine_free_buffer(test_engine, compressed_data);
    compression_engine_free_buffer(test_engine, decompressed_data);
}

/**
 * @brief Test DEFLATE compression
 */
void test_deflate_compression(void) {
    /* Configure for DEFLATE compression */
    bool result = compression_engine_set_algorithm(test_engine, COMPRESSION_ALGORITHM_DEFLATE);
    TEST_ASSERT_TRUE(result);
    
    result = compression_engine_set_level(test_engine, COMPRESSION_LEVEL_BALANCED);
    TEST_ASSERT_TRUE(result);
    
    /* Create highly compressible test data */
    const size_t input_size = 1024;
    uint8_t* input_data = malloc(input_size);
    TEST_ASSERT_NOT_NULL(input_data);
    
    /* Fill with repeating pattern */
    for (size_t i = 0; i < input_size; i++) {
        input_data[i] = (uint8_t)(i % 16);
    }
    
    /* Compress data */
    size_t compressed_size;
    uint8_t* compressed_data = compression_engine_compress(test_engine,
                                                          input_data,
                                                          input_size,
                                                          &compressed_size);
    
    TEST_ASSERT_NOT_NULL(compressed_data);
    TEST_ASSERT_LESS_THAN(input_size, compressed_size);  /* Should be compressed */
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    /* Verify compression ratio */
    float compression_ratio = (float)compressed_size / (float)input_size;
    TEST_ASSERT_LESS_THAN(0.8f, compression_ratio);  /* At least 20% compression */
    
    /* Decompress data */
    size_t decompressed_size;
    uint8_t* decompressed_data = compression_engine_decompress(test_engine,
                                                              compressed_data,
                                                              compressed_size,
                                                              &decompressed_size);
    
    TEST_ASSERT_NOT_NULL(decompressed_data);
    TEST_ASSERT_EQUAL_size_t(input_size, decompressed_size);
    TEST_ASSERT_EQUAL_MEMORY(input_data, decompressed_data, input_size);
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    /* Cleanup */
    free(input_data);
    compression_engine_free_buffer(test_engine, compressed_data);
    compression_engine_free_buffer(test_engine, decompressed_data);
}

/**
 * @brief Test GZIP compression
 */
void test_gzip_compression(void) {
    /* Configure for GZIP compression */
    bool result = compression_engine_set_algorithm(test_engine, COMPRESSION_ALGORITHM_GZIP);
    TEST_ASSERT_TRUE(result);
    
    /* Test with typical SMOF file content */
    uint8_t smof_like_data[] = {
        /* SMOF header */
        0x53, 0x4D, 0x4F, 0x46,  /* Magic */
        0x01, 0x00,              /* Version */
        0x00, 0x00,              /* Flags */
        0x00, 0x10, 0x00, 0x00,  /* Entry point */
        0x02, 0x00,              /* Section count */
        0x05, 0x00,              /* Symbol count */
        /* Repeated zeros (common in object files) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    size_t input_size = sizeof(smof_like_data);
    
    /* Compress */
    size_t compressed_size;
    uint8_t* compressed_data = compression_engine_compress(test_engine,
                                                          smof_like_data,
                                                          input_size,
                                                          &compressed_size);
    
    TEST_ASSERT_NOT_NULL(compressed_data);
    TEST_ASSERT_LESS_THAN(input_size, compressed_size);  /* Should compress zeros well */
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
    
    /* Verify GZIP header */
    TEST_ASSERT_EQUAL_HEX8(0x1F, compressed_data[0]);  /* GZIP magic 1 */
    TEST_ASSERT_EQUAL_HEX8(0x8B, compressed_data[1]);  /* GZIP magic 2 */
    TEST_ASSERT_EQUAL_HEX8(0x08, compressed_data[2]);  /* Compression method (DEFLATE) */
    
    /* Decompress and verify */
    size_t decompressed_size;
    uint8_t* decompressed_data = compression_engine_decompress(test_engine,
                                                              compressed_data,
                                                              compressed_size,
                                                              &decompressed_size);
    
    TEST_ASSERT_NOT_NULL(decompressed_data);
    TEST_ASSERT_EQUAL_size_t(input_size, decompressed_size);
    TEST_ASSERT_EQUAL_MEMORY(smof_like_data, decompressed_data, input_size);
    
    /* Cleanup */
    compression_engine_free_buffer(test_engine, compressed_data);
    compression_engine_free_buffer(test_engine, decompressed_data);
}

/**
 * @brief Test compression level effects
 */
void test_compression_level_effects(void) {
    /* Set up test data */
    const size_t input_size = 2048;
    uint8_t* test_data = malloc(input_size);
    TEST_ASSERT_NOT_NULL(test_data);
    
    /* Create semi-compressible data */
    for (size_t i = 0; i < input_size; i++) {
        test_data[i] = (uint8_t)((i * 7) % 256);  /* Pseudo-random pattern */
    }
    
    compression_engine_set_algorithm(test_engine, COMPRESSION_ALGORITHM_DEFLATE);
    
    /* Test different compression levels */
    compression_level_t levels[] = {
        COMPRESSION_LEVEL_FASTEST,
        COMPRESSION_LEVEL_FAST,
        COMPRESSION_LEVEL_BALANCED,
        COMPRESSION_LEVEL_BEST
    };
    
    size_t compressed_sizes[sizeof(levels) / sizeof(levels[0])];
    
    for (size_t i = 0; i < sizeof(levels) / sizeof(levels[0]); i++) {
        bool result = compression_engine_set_level(test_engine, levels[i]);
        TEST_ASSERT_TRUE(result);
        
        size_t compressed_size;
        uint8_t* compressed_data = compression_engine_compress(test_engine,
                                                              test_data,
                                                              input_size,
                                                              &compressed_size);
        
        TEST_ASSERT_NOT_NULL(compressed_data);
        compressed_sizes[i] = compressed_size;
        
        /* Verify decompression works */
        size_t decompressed_size;
        uint8_t* decompressed_data = compression_engine_decompress(test_engine,
                                                                  compressed_data,
                                                                  compressed_size,
                                                                  &decompressed_size);
        
        TEST_ASSERT_NOT_NULL(decompressed_data);
        TEST_ASSERT_EQUAL_size_t(input_size, decompressed_size);
        TEST_ASSERT_EQUAL_MEMORY(test_data, decompressed_data, input_size);
        
        compression_engine_free_buffer(test_engine, compressed_data);
        compression_engine_free_buffer(test_engine, decompressed_data);
    }
    
    /* Generally, higher compression levels should produce smaller output */
    /* (though this isn't guaranteed for all data) */
    TEST_ASSERT_GREATER_OR_EQUAL(compressed_sizes[3], compressed_sizes[0]);  /* BEST <= FASTEST */
    
    free(test_data);
}

/**
 * @brief Test compression with random data (incompressible)
 */
void test_incompressible_data(void) {
    /* Generate pseudo-random incompressible data */
    const size_t input_size = 1024;
    uint8_t* random_data = malloc(input_size);
    TEST_ASSERT_NOT_NULL(random_data);
    
    /* Simple pseudo-random generator */
    uint32_t seed = 12345;
    for (size_t i = 0; i < input_size; i++) {
        seed = seed * 1103515245 + 12345;
        random_data[i] = (uint8_t)(seed >> 16);
    }
    
    compression_engine_set_algorithm(test_engine, COMPRESSION_ALGORITHM_DEFLATE);
    compression_engine_set_level(test_engine, COMPRESSION_LEVEL_BALANCED);
    
    /* Compress random data */
    size_t compressed_size;
    uint8_t* compressed_data = compression_engine_compress(test_engine,
                                                          random_data,
                                                          input_size,
                                                          &compressed_size);
    
    TEST_ASSERT_NOT_NULL(compressed_data);
    
    /* Incompressible data might actually expand slightly */
    TEST_ASSERT_GREATER_OR_EQUAL(compressed_size, input_size * 0.9);  /* At most 10% shrinkage */
    TEST_ASSERT_LESS_THAN(input_size * 1.2, compressed_size);        /* At most 20% expansion */
    
    /* Verify decompression */
    size_t decompressed_size;
    uint8_t* decompressed_data = compression_engine_decompress(test_engine,
                                                              compressed_data,
                                                              compressed_size,
                                                              &decompressed_size);
    
    TEST_ASSERT_NOT_NULL(decompressed_data);
    TEST_ASSERT_EQUAL_size_t(input_size, decompressed_size);
    TEST_ASSERT_EQUAL_MEMORY(random_data, decompressed_data, input_size);
    
    /* Cleanup */
    free(random_data);
    compression_engine_free_buffer(test_engine, compressed_data);
    compression_engine_free_buffer(test_engine, decompressed_data);
}

/**
 * @brief Test compression error handling
 */
void test_compression_error_handling(void) {
    /* Test compression with NULL data */
    size_t compressed_size;
    uint8_t* compressed_data = compression_engine_compress(test_engine,
                                                          NULL,
                                                          100,
                                                          &compressed_size);
    
    TEST_ASSERT_NULL(compressed_data);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
    
    error_context_clear(test_error_ctx);
    
    /* Test compression with zero size */
    uint8_t test_data[] = {0x01, 0x02, 0x03};
    compressed_data = compression_engine_compress(test_engine,
                                                 test_data,
                                                 0,
                                                 &compressed_size);
    
    /* May succeed (empty compression) or fail */
    if (compressed_data == NULL) {
        TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
        error_context_clear(test_error_ctx);
    } else {
        TEST_ASSERT_EQUAL_size_t(0, compressed_size);
        compression_engine_free_buffer(test_engine, compressed_data);
    }
    
    /* Test decompression with invalid data */
    uint8_t invalid_data[] = {0xFF, 0xFE, 0xFD, 0xFC};
    size_t decompressed_size;
    uint8_t* decompressed_data = compression_engine_decompress(test_engine,
                                                              invalid_data,
                                                              sizeof(invalid_data),
                                                              &decompressed_size);
    
    TEST_ASSERT_NULL(decompressed_data);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Test compression buffer management
 */
void test_compression_buffer_management(void) {
    /* Test that buffers can be reused */
    uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    
    compression_engine_set_algorithm(test_engine, COMPRESSION_ALGORITHM_NONE);
    
    /* First compression */
    size_t size1;
    uint8_t* buffer1 = compression_engine_compress(test_engine,
                                                  test_data,
                                                  sizeof(test_data),
                                                  &size1);
    TEST_ASSERT_NOT_NULL(buffer1);
    
    /* Second compression */
    size_t size2;
    uint8_t* buffer2 = compression_engine_compress(test_engine,
                                                  test_data,
                                                  sizeof(test_data),
                                                  &size2);
    TEST_ASSERT_NOT_NULL(buffer2);
    
    /* Buffers should be different (no reuse for safety) */
    TEST_ASSERT_NOT_EQUAL(buffer1, buffer2);
    
    /* Both should contain same data */
    TEST_ASSERT_EQUAL_size_t(size1, size2);
    TEST_ASSERT_EQUAL_MEMORY(buffer1, buffer2, size1);
    
    /* Test free operations */
    compression_engine_free_buffer(test_engine, buffer1);
    compression_engine_free_buffer(test_engine, buffer2);
    
    /* Freeing NULL should not crash */
    compression_engine_free_buffer(test_engine, NULL);
}

/**
 * @brief Test compression statistics
 */
void test_compression_statistics(void) {
    /* Create test data with known pattern */
    const size_t input_size = 1000;
    uint8_t* test_data = malloc(input_size);
    TEST_ASSERT_NOT_NULL(test_data);
    
    /* Half zeros, half pattern - should compress well */
    memset(test_data, 0x00, input_size / 2);
    for (size_t i = input_size / 2; i < input_size; i++) {
        test_data[i] = (uint8_t)(i % 8);
    }
    
    compression_engine_set_algorithm(test_engine, COMPRESSION_ALGORITHM_DEFLATE);
    
    /* Reset statistics */
    compression_engine_reset_statistics(test_engine);
    
    /* Perform compression */
    size_t compressed_size;
    uint8_t* compressed_data = compression_engine_compress(test_engine,
                                                          test_data,
                                                          input_size,
                                                          &compressed_size);
    TEST_ASSERT_NOT_NULL(compressed_data);
    
    /* Get statistics */
    compression_stats_t stats = compression_engine_get_statistics(test_engine);
    
    TEST_ASSERT_EQUAL_size_t(1, stats.compression_count);
    TEST_ASSERT_EQUAL_size_t(0, stats.decompression_count);
    TEST_ASSERT_EQUAL_size_t(input_size, stats.total_input_bytes);
    TEST_ASSERT_EQUAL_size_t(compressed_size, stats.total_output_bytes);
    TEST_ASSERT_GREATER_THAN(0.0f, stats.average_compression_ratio);
    
    /* Perform decompression */
    size_t decompressed_size;
    uint8_t* decompressed_data = compression_engine_decompress(test_engine,
                                                              compressed_data,
                                                              compressed_size,
                                                              &decompressed_size);
    TEST_ASSERT_NOT_NULL(decompressed_data);
    
    /* Check updated statistics */
    stats = compression_engine_get_statistics(test_engine);
    TEST_ASSERT_EQUAL_size_t(1, stats.compression_count);
    TEST_ASSERT_EQUAL_size_t(1, stats.decompression_count);
    
    /* Cleanup */
    free(test_data);
    compression_engine_free_buffer(test_engine, compressed_data);
    compression_engine_free_buffer(test_engine, decompressed_data);
}

/**
 * @brief Test compression with NULL parameters
 */
void test_compression_null_parameters(void) {
    /* Test operations with NULL engine */
    TEST_ASSERT_FALSE(compression_engine_set_algorithm(NULL, COMPRESSION_ALGORITHM_DEFLATE));
    TEST_ASSERT_FALSE(compression_engine_set_level(NULL, COMPRESSION_LEVEL_BALANCED));
    
    uint8_t test_data[] = {0x01, 0x02, 0x03};
    size_t size;
    
    uint8_t* result = compression_engine_compress(NULL, test_data, sizeof(test_data), &size);
    TEST_ASSERT_NULL(result);
    
    result = compression_engine_decompress(NULL, test_data, sizeof(test_data), &size);
    TEST_ASSERT_NULL(result);
    
    /* Test with NULL size pointer */
    result = compression_engine_compress(test_engine, test_data, sizeof(test_data), NULL);
    TEST_ASSERT_NULL(result);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Main test runner function
 */
int main(void) {
    UNITY_BEGIN();
    
    /* Basic functionality tests */
    RUN_TEST(test_compression_engine_lifecycle);
    RUN_TEST(test_compression_algorithm_configuration);
    RUN_TEST(test_compression_level_configuration);
    
    /* Compression algorithm tests */
    RUN_TEST(test_no_compression);
    RUN_TEST(test_deflate_compression);
    RUN_TEST(test_gzip_compression);
    
    /* Compression behavior tests */
    RUN_TEST(test_compression_level_effects);
    RUN_TEST(test_incompressible_data);
    
    /* Error handling tests */
    RUN_TEST(test_compression_error_handling);
    RUN_TEST(test_compression_buffer_management);
    
    /* Advanced functionality tests */
    RUN_TEST(test_compression_statistics);
    
    /* Edge case tests */
    RUN_TEST(test_compression_null_parameters);
    
    return UNITY_END();
}
