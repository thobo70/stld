/**
 * @file test_memory.c
 * @brief Unit tests for memory management module
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * Comprehensive unit tests for the memory pool allocator and related
 * memory management functions. Tests cover normal operation, edge cases,
 * error conditions, and memory alignment requirements.
 */

#include "unity.h"
#include "memory.h"
#include <stdint.h>
#include <string.h>
#include <limits.h>

/* Test fixture data */
static memory_pool_t* test_pool;
static const size_t TEST_POOL_SIZE = 1024;

/**
 * @brief Setup function called before each test
 */
void setUp(void) {
    test_pool = memory_pool_create(TEST_POOL_SIZE);
    TEST_ASSERT_NOT_NULL_MESSAGE(test_pool, "Failed to create test memory pool");
}

/**
 * @brief Teardown function called after each test
 */
void tearDown(void) {
    if (test_pool != NULL) {
        memory_pool_destroy(test_pool);
        test_pool = NULL;
    }
}

/**
 * @brief Test memory pool creation with valid parameters
 */
void test_memory_pool_create_valid(void) {
    memory_pool_t* pool = memory_pool_create(512);
    
    TEST_ASSERT_NOT_NULL(pool);
    TEST_ASSERT_EQUAL_size_t(512, memory_pool_get_size(pool));
    TEST_ASSERT_EQUAL_size_t(0, memory_pool_get_used(pool));
    TEST_ASSERT_EQUAL_size_t(512, memory_pool_get_available(pool));
    
    memory_pool_destroy(pool);
}

/**
 * @brief Test memory pool creation with invalid parameters
 */
void test_memory_pool_create_invalid(void) {
    /* Zero size should fail */
    memory_pool_t* pool = memory_pool_create(0);
    TEST_ASSERT_NULL(pool);
    
    /* Very large size might fail (system dependent) */
    pool = memory_pool_create(SIZE_MAX);
    if (pool != NULL) {
        memory_pool_destroy(pool);
    }
    /* Note: This test is informational, may succeed on some systems */
}

/**
 * @brief Test basic memory allocation
 */
void test_memory_pool_alloc_basic(void) {
    void* ptr1 = memory_pool_alloc(test_pool, 64);
    void* ptr2 = memory_pool_alloc(test_pool, 128);
    void* ptr3 = memory_pool_alloc(test_pool, 32);
    
    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    TEST_ASSERT_NOT_NULL(ptr3);
    
    /* Pointers should be different */
    TEST_ASSERT_NOT_EQUAL(ptr1, ptr2);
    TEST_ASSERT_NOT_EQUAL(ptr2, ptr3);
    TEST_ASSERT_NOT_EQUAL(ptr1, ptr3);
    
    /* Check memory usage */
    size_t used = memory_pool_get_used(test_pool);
    TEST_ASSERT_GREATER_OR_EQUAL(64 + 128 + 32, used);
    TEST_ASSERT_LESS_THAN(TEST_POOL_SIZE, used);
}

/**
 * @brief Test memory alignment requirements
 */
void test_memory_pool_alignment(void) {
    void* ptr1 = memory_pool_alloc(test_pool, 1);
    void* ptr2 = memory_pool_alloc(test_pool, 3);
    void* ptr3 = memory_pool_alloc(test_pool, 7);
    
    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    TEST_ASSERT_NOT_NULL(ptr3);
    
    /* Check alignment (should be aligned to pointer size) */
    uintptr_t alignment = sizeof(void*);
    TEST_ASSERT_EQUAL_INT(0, ((uintptr_t)ptr1) % alignment);
    TEST_ASSERT_EQUAL_INT(0, ((uintptr_t)ptr2) % alignment);
    TEST_ASSERT_EQUAL_INT(0, ((uintptr_t)ptr3) % alignment);
}

/**
 * @brief Test memory pool exhaustion
 */
void test_memory_pool_exhaustion(void) {
    /* Allocate until exhaustion */
    void* ptrs[32];
    size_t count = 0;
    
    for (size_t i = 0; i < 32; i++) {
        ptrs[i] = memory_pool_alloc(test_pool, 64);
        if (ptrs[i] != NULL) {
            count++;
        } else {
            break;
        }
    }
    
    TEST_ASSERT_GREATER_THAN(0, count);
    TEST_ASSERT_LESS_THAN(32, count); /* Should exhaust before 32 * 64 = 2048 bytes */
    
    /* Next allocation should fail */
    void* overflow = memory_pool_alloc(test_pool, 1);
    TEST_ASSERT_NULL(overflow);
    
    /* Pool should be nearly full */
    size_t available = memory_pool_get_available(test_pool);
    TEST_ASSERT_LESS_THAN(64, available); /* Less than one more allocation */
}

/**
 * @brief Test memory pool reset functionality
 */
void test_memory_pool_reset(void) {
    /* Allocate some memory */
    void* ptr1 = memory_pool_alloc(test_pool, 256);
    void* ptr2 = memory_pool_alloc(test_pool, 128);
    
    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    
    size_t used_before = memory_pool_get_used(test_pool);
    TEST_ASSERT_GREATER_THAN(256 + 128, used_before);
    
    /* Reset pool */
    memory_pool_reset(test_pool);
    
    size_t used_after = memory_pool_get_used(test_pool);
    TEST_ASSERT_EQUAL_size_t(0, used_after);
    TEST_ASSERT_EQUAL_size_t(TEST_POOL_SIZE, memory_pool_get_available(test_pool));
    
    /* Should be able to allocate again */
    void* ptr3 = memory_pool_alloc(test_pool, 256);
    TEST_ASSERT_NOT_NULL(ptr3);
    TEST_ASSERT_EQUAL_PTR(ptr1, ptr3); /* Should get same address */
}

/**
 * @brief Test memory pool with zero-size allocations
 */
void test_memory_pool_alloc_zero_size(void) {
    void* ptr = memory_pool_alloc(test_pool, 0);
    
    /* Implementation defined - could return NULL or valid pointer */
    if (ptr != NULL) {
        /* If returned, should be properly aligned */
        uintptr_t alignment = sizeof(void*);
        TEST_ASSERT_EQUAL_INT(0, ((uintptr_t)ptr) % alignment);
    }
    
    /* Pool usage should not increase significantly */
    size_t used = memory_pool_get_used(test_pool);
    TEST_ASSERT_LESS_THAN(sizeof(void*) * 2, used);
}

/**
 * @brief Test memory pool with NULL parameters
 */
void test_memory_pool_null_parameters(void) {
    /* NULL pool should fail gracefully */
    void* ptr = memory_pool_alloc(NULL, 64);
    TEST_ASSERT_NULL(ptr);
    
    size_t size = memory_pool_get_size(NULL);
    TEST_ASSERT_EQUAL_size_t(0, size);
    
    size_t used = memory_pool_get_used(NULL);
    TEST_ASSERT_EQUAL_size_t(0, used);
    
    size_t available = memory_pool_get_available(NULL);
    TEST_ASSERT_EQUAL_size_t(0, available);
    
    /* Reset should not crash */
    memory_pool_reset(NULL);
    
    /* Destroy should not crash */
    memory_pool_destroy(NULL);
}

/**
 * @brief Test memory pool statistics accuracy
 */
void test_memory_pool_statistics(void) {
    size_t initial_size = memory_pool_get_size(test_pool);
    size_t initial_used = memory_pool_get_used(test_pool);
    size_t initial_available = memory_pool_get_available(test_pool);
    
    TEST_ASSERT_EQUAL_size_t(TEST_POOL_SIZE, initial_size);
    TEST_ASSERT_EQUAL_size_t(0, initial_used);
    TEST_ASSERT_EQUAL_size_t(TEST_POOL_SIZE, initial_available);
    TEST_ASSERT_EQUAL_size_t(initial_size, initial_used + initial_available);
    
    /* Allocate memory and check statistics */
    void* ptr = memory_pool_alloc(test_pool, 100);
    TEST_ASSERT_NOT_NULL(ptr);
    
    size_t size_after = memory_pool_get_size(test_pool);
    size_t used_after = memory_pool_get_used(test_pool);
    size_t available_after = memory_pool_get_available(test_pool);
    
    TEST_ASSERT_EQUAL_size_t(TEST_POOL_SIZE, size_after);
    TEST_ASSERT_GREATER_OR_EQUAL(100, used_after);
    TEST_ASSERT_EQUAL_size_t(size_after, used_after + available_after);
}

/**
 * @brief Test memory pool copy functionality
 */
void test_memory_pool_copy(void) {
    /* Create source pool with data */
    memory_pool_t* src_pool = memory_pool_create(512);
    TEST_ASSERT_NOT_NULL(src_pool);
    
    void* src_ptr1 = memory_pool_alloc(src_pool, 64);
    void* src_ptr2 = memory_pool_alloc(src_pool, 128);
    TEST_ASSERT_NOT_NULL(src_ptr1);
    TEST_ASSERT_NOT_NULL(src_ptr2);
    
    /* Fill with test data */
    memset(src_ptr1, 0xAA, 64);
    memset(src_ptr2, 0xBB, 128);
    
    size_t src_used = memory_pool_get_used(src_pool);
    
    /* Copy to destination pool */
    memory_pool_copy(test_pool, src_pool, src_used);
    
    /* Verify copy */
    TEST_ASSERT_EQUAL_size_t(src_used, memory_pool_get_used(test_pool));
    
    /* Get equivalent pointers in destination */
    memory_pool_reset(test_pool);
    void* dst_ptr1 = memory_pool_alloc(test_pool, 64);
    void* dst_ptr2 = memory_pool_alloc(test_pool, 128);
    
    TEST_ASSERT_NOT_NULL(dst_ptr1);
    TEST_ASSERT_NOT_NULL(dst_ptr2);
    
    /* Verify data integrity */
    uint8_t* dst_data1 = (uint8_t*)dst_ptr1;
    uint8_t* dst_data2 = (uint8_t*)dst_ptr2;
    
    for (size_t i = 0; i < 64; i++) {
        TEST_ASSERT_EQUAL_HEX8(0xAA, dst_data1[i]);
    }
    for (size_t i = 0; i < 128; i++) {
        TEST_ASSERT_EQUAL_HEX8(0xBB, dst_data2[i]);
    }
    
    memory_pool_destroy(src_pool);
}

/**
 * @brief Test memory pool with large allocations
 */
void test_memory_pool_large_allocation(void) {
    /* Try to allocate more than pool size */
    void* ptr = memory_pool_alloc(test_pool, TEST_POOL_SIZE + 1);
    TEST_ASSERT_NULL(ptr);
    
    /* Try to allocate exactly pool size */
    ptr = memory_pool_alloc(test_pool, TEST_POOL_SIZE);
    /* May succeed or fail depending on overhead */
    
    if (ptr != NULL) {
        /* If successful, no more allocations should be possible */
        void* ptr2 = memory_pool_alloc(test_pool, 1);
        TEST_ASSERT_NULL(ptr2);
    }
}

/**
 * @brief Test memory pool fragmentation behavior
 */
void test_memory_pool_fragmentation(void) {
    /* Memory pool should be contiguous, no fragmentation in linear allocator */
    void* ptrs[10];
    
    /* Allocate multiple small blocks */
    for (int i = 0; i < 10; i++) {
        ptrs[i] = memory_pool_alloc(test_pool, 32);
        TEST_ASSERT_NOT_NULL(ptrs[i]);
    }
    
    /* Verify pointers are in ascending order (contiguous allocation) */
    for (int i = 1; i < 10; i++) {
        TEST_ASSERT_GREATER_THAN(ptrs[i-1], ptrs[i]);
        
        /* Check that gap between allocations is reasonable (aligned) */
        uintptr_t gap = (uintptr_t)ptrs[i] - (uintptr_t)ptrs[i-1];
        TEST_ASSERT_GREATER_OR_EQUAL(32, gap);
        TEST_ASSERT_LESS_THAN(64, gap); /* Shouldn't have excessive overhead */
    }
}

/**
 * @brief Main test runner function
 */
int main(void) {
    UNITY_BEGIN();
    
    /* Basic functionality tests */
    RUN_TEST(test_memory_pool_create_valid);
    RUN_TEST(test_memory_pool_create_invalid);
    RUN_TEST(test_memory_pool_alloc_basic);
    RUN_TEST(test_memory_pool_alignment);
    
    /* Resource management tests */
    RUN_TEST(test_memory_pool_exhaustion);
    RUN_TEST(test_memory_pool_reset);
    RUN_TEST(test_memory_pool_statistics);
    
    /* Edge case tests */
    RUN_TEST(test_memory_pool_alloc_zero_size);
    RUN_TEST(test_memory_pool_null_parameters);
    RUN_TEST(test_memory_pool_large_allocation);
    
    /* Advanced functionality tests */
    RUN_TEST(test_memory_pool_copy);
    RUN_TEST(test_memory_pool_fragmentation);
    
    return UNITY_END();
}
