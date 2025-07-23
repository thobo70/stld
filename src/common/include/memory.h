/* src/common/include/memory.h */
#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file memory.h
 * @brief Memory management for embedded systems
 * @details C99 compliant memory pool allocator for deterministic allocation
 */

/* Memory pool configuration */
#define MEMORY_POOL_ALIGN 8
#define MEMORY_POOL_MIN_SIZE 64
#define MEMORY_POOL_MAX_SIZE (1024 * 1024)  /* 1MB max */

/* Memory pool statistics */
typedef struct memory_pool_stats {
    size_t total_size;        /* Total pool size */
    size_t used_size;         /* Currently used */
    size_t peak_used;         /* Peak usage */
    size_t allocations;       /* Number of allocations */
    size_t deallocations;     /* Number of deallocations */
    size_t alignment;         /* Pool alignment */
} memory_pool_stats_t;

/* Forward declaration */
typedef struct memory_pool memory_pool_t;

/* Memory pool operations */
memory_pool_t* memory_pool_create(size_t size);
void memory_pool_destroy(memory_pool_t* pool);
void* memory_pool_alloc(memory_pool_t* pool, size_t size);
void* memory_pool_calloc(memory_pool_t* pool, size_t count, size_t size);
void memory_pool_free(memory_pool_t* pool, void* ptr);
void memory_pool_reset(memory_pool_t* pool);

/* Memory pool information */
size_t memory_pool_get_size(const memory_pool_t* pool);
size_t memory_pool_get_used(const memory_pool_t* pool);
size_t memory_pool_get_available(const memory_pool_t* pool);
void memory_pool_get_stats(const memory_pool_t* pool, memory_pool_stats_t* stats);

/* Memory utilities */
void* memory_align_ptr(void* ptr, size_t alignment);
size_t memory_align_size(size_t size, size_t alignment);
bool memory_is_aligned(const void* ptr, size_t alignment);

/* C99 inline utility functions */
static inline bool memory_pool_is_valid(const memory_pool_t* pool) {
    return pool != NULL;
}

static inline bool memory_pool_can_alloc(const memory_pool_t* pool, size_t size) {
    return pool != NULL && memory_pool_get_available(pool) >= size;
}

/* Memory debugging support */
#ifdef DEBUG
typedef struct memory_debug_info {
    const char* file;
    int line;
    size_t size;
    void* ptr;
} memory_debug_info_t;

#define memory_pool_alloc_debug(pool, size) \
    memory_pool_alloc_with_debug(pool, size, __FILE__, __LINE__)

void* memory_pool_alloc_with_debug(memory_pool_t* pool, size_t size, 
                                  const char* file, int line);
void memory_pool_dump_allocations(const memory_pool_t* pool);
#endif /* DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_H_INCLUDED */
