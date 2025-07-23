/* tests/test_memory.c */
#include "unity.h"
#include "memory.h"
#include <stdio.h>
#include <string.h>

/**
 * @file test_memory.c
 * @brief Unit tests for memory pool allocator
 * @details Tests memory pool allocation, deallocation, and alignment
 */

/* Test pool for testing */
static memory_pool_t* test_pool;

void setUp(void) {
    /* Create test pool before each test */
    test_pool = memory_pool_create(1024);
}

void tearDown(void) {
    /* Clean up after each test */
    if (test_pool) {
        memory_pool_destroy(test_pool);
        test_pool = NULL;
    }
}

void test_memory_pool_create(void) {
    memory_pool_t* pool = memory_pool_create(512);
    
    TEST_ASSERT_NOT_NULL(pool);
    TEST_ASSERT_EQUAL_UINT(512, memory_pool_get_size(pool));
    TEST_ASSERT_EQUAL_UINT(0, memory_pool_get_used(pool));
    TEST_ASSERT_TRUE(memory_pool_get_available(pool) > 0);
    
    memory_pool_destroy(pool);
}

void test_memory_pool_create_invalid_size(void) {
    memory_pool_t* pool1 = memory_pool_create(0);
    memory_pool_t* pool2 = memory_pool_create(MEMORY_POOL_MAX_SIZE + 1);
    
    TEST_ASSERT_NULL(pool1);
    TEST_ASSERT_NULL(pool2);
}

void test_memory_pool_alloc(void) {
    void* ptr1 = memory_pool_alloc(test_pool, 64);
    void* ptr2 = memory_pool_alloc(test_pool, 128);
    
    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    TEST_ASSERT_TRUE(ptr1 != ptr2);
    
    /* Check alignment */
    TEST_ASSERT_TRUE(memory_is_aligned(ptr1, MEMORY_POOL_ALIGN));
    TEST_ASSERT_TRUE(memory_is_aligned(ptr2, MEMORY_POOL_ALIGN));
}

void test_memory_pool_alloc_null_pool(void) {
    void* ptr = memory_pool_alloc(NULL, 64);
    
    TEST_ASSERT_NULL(ptr);
}

void test_memory_pool_alloc_zero_size(void) {
    void* ptr = memory_pool_alloc(test_pool, 0);
    
    TEST_ASSERT_NULL(ptr);
}

void test_memory_pool_alloc_too_large(void) {
    void* ptr = memory_pool_alloc(test_pool, memory_pool_get_size(test_pool) + 1);
    
    TEST_ASSERT_NULL(ptr);
}

void test_memory_pool_free(void) {
    void* ptr = memory_pool_alloc(test_pool, 64);
    TEST_ASSERT_NOT_NULL(ptr);
    
    size_t used_before = memory_pool_get_used(test_pool);
    memory_pool_free(test_pool, ptr);
    
    /* Memory should be available for reuse */
    void* ptr2 = memory_pool_alloc(test_pool, 64);
    TEST_ASSERT_NOT_NULL(ptr2);
}

void test_memory_pool_free_null_ptr(void) {
    /* Should not crash */
    memory_pool_free(test_pool, NULL);
}

void test_memory_pool_free_null_pool(void) {
    void* ptr = memory_pool_alloc(test_pool, 64);
    
    /* Should not crash */
    memory_pool_free(NULL, ptr);
}

void test_memory_pool_reset(void) {
    void* ptr1 = memory_pool_alloc(test_pool, 64);
    void* ptr2 = memory_pool_alloc(test_pool, 128);
    
    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    TEST_ASSERT_TRUE(memory_pool_get_used(test_pool) > 0);
    
    memory_pool_reset(test_pool);
    
    TEST_ASSERT_EQUAL_UINT(0, memory_pool_get_used(test_pool));
    
    /* Should be able to allocate again */
    void* ptr3 = memory_pool_alloc(test_pool, 256);
    TEST_ASSERT_NOT_NULL(ptr3);
}

void test_memory_pool_calloc(void) {
    uint32_t* array = (uint32_t*)memory_pool_calloc(test_pool, 10, sizeof(uint32_t));
    
    TEST_ASSERT_NOT_NULL(array);
    
    /* Check that memory is zeroed */
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL_UINT(0, array[i]);
    }
}

void test_memory_pool_get_stats(void) {
    memory_pool_stats_t stats;
    
    void* ptr1 = memory_pool_alloc(test_pool, 64);
    void* ptr2 = memory_pool_alloc(test_pool, 128);
    
    memory_pool_get_stats(test_pool, &stats);
    
    TEST_ASSERT_EQUAL_UINT(1024, stats.total_size);
    TEST_ASSERT_TRUE(stats.used_size > 0);
    TEST_ASSERT_TRUE(stats.allocations >= 2);
    TEST_ASSERT_EQUAL_UINT(MEMORY_POOL_ALIGN, stats.alignment);
}

void test_memory_pool_fragmentation(void) {
    /* Allocate several blocks */
    void* ptrs[5];
    for (int i = 0; i < 5; i++) {
        ptrs[i] = memory_pool_alloc(test_pool, 32);
        TEST_ASSERT_NOT_NULL(ptrs[i]);
    }
    
    /* Deallocate every other block */
    memory_pool_free(test_pool, ptrs[1]);
    memory_pool_free(test_pool, ptrs[3]);
    
    /* Should still be able to allocate small blocks */
    void* small_ptr = memory_pool_alloc(test_pool, 16);
    TEST_ASSERT_NOT_NULL(small_ptr);
}

void test_memory_utility_functions(void) {
    /* Test alignment utilities */
    char buffer[100];
    void* aligned = memory_align_ptr(buffer, 8);
    TEST_ASSERT_NOT_NULL(aligned);
    TEST_ASSERT_TRUE(memory_is_aligned(aligned, 8));
    
    size_t aligned_size = memory_align_size(10, 8);
    TEST_ASSERT_TRUE(aligned_size >= 10);
    TEST_ASSERT_EQUAL_UINT(0, aligned_size % 8);
}

void test_memory_pool_inline_functions(void) {
    TEST_ASSERT_TRUE(memory_pool_is_valid(test_pool));
    TEST_ASSERT_FALSE(memory_pool_is_valid(NULL));
    
    TEST_ASSERT_TRUE(memory_pool_can_alloc(test_pool, 64));
    TEST_ASSERT_FALSE(memory_pool_can_alloc(test_pool, memory_pool_get_size(test_pool) + 1));
    TEST_ASSERT_FALSE(memory_pool_can_alloc(NULL, 64));
}

int test_memory_main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_memory_pool_create);
    RUN_TEST(test_memory_pool_create_invalid_size);
    RUN_TEST(test_memory_pool_alloc);
    RUN_TEST(test_memory_pool_alloc_null_pool);
    RUN_TEST(test_memory_pool_alloc_zero_size);
    RUN_TEST(test_memory_pool_alloc_too_large);
    RUN_TEST(test_memory_pool_free);
    RUN_TEST(test_memory_pool_free_null_ptr);
    RUN_TEST(test_memory_pool_free_null_pool);
    RUN_TEST(test_memory_pool_reset);
    RUN_TEST(test_memory_pool_calloc);
    RUN_TEST(test_memory_pool_get_stats);
    RUN_TEST(test_memory_pool_fragmentation);
    RUN_TEST(test_memory_utility_functions);
    RUN_TEST(test_memory_pool_inline_functions);
    
    return UNITY_END();
}

int main(void) {
    return test_memory_main();
}
