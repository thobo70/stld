/* src/common/memory.c */
#include "memory.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * @file memory.c
 * @brief Memory pool implementation
 * @details C99 compliant memory management for embedded systems
 */

/* Memory pool structure with C99 flexible array member */
struct memory_pool {
    size_t size;                    /* Total pool size */
    size_t used;                    /* Currently used bytes */
    size_t peak_used;               /* Peak usage */
    size_t allocations;             /* Number of allocations */
    size_t deallocations;           /* Number of deallocations */
    size_t alignment;               /* Pool alignment */
    uint8_t* free_ptr;              /* Next free position */
    uint8_t data[];                 /* C99 flexible array member */
};

memory_pool_t* memory_pool_create(size_t size) {
    memory_pool_t* pool;
    
    if (size < MEMORY_POOL_MIN_SIZE || size > MEMORY_POOL_MAX_SIZE) {
        ERROR_REPORT_ERROR(ERROR_INVALID_ARGUMENT, "Invalid pool size");
        return NULL;
    }
    
    pool = malloc(sizeof(memory_pool_t) + size);
    if (pool == NULL) {
        ERROR_REPORT_ERROR(ERROR_OUT_OF_MEMORY, "Failed to allocate memory pool");
        return NULL;
    }
    
    /* C99 designated initializer style */
    *pool = (memory_pool_t) {
        .size = size,
        .used = 0,
        .peak_used = 0,
        .allocations = 0,
        .deallocations = 0,
        .alignment = MEMORY_POOL_ALIGN
    };
    
    pool->free_ptr = pool->data;
    
    return pool;
}

void memory_pool_destroy(memory_pool_t* pool) {
    if (pool != NULL) {
        /* Clear sensitive data before freeing */
        memset(pool, 0, sizeof(memory_pool_t) + pool->size);
        free(pool);
    }
}

void* memory_pool_alloc(memory_pool_t* pool, size_t size) {
    size_t aligned_size;
    void* ptr;
    
    if (pool == NULL || size == 0) {
        return NULL;
    }
    
    /* Align size to pool alignment */
    aligned_size = memory_align_size(size, pool->alignment);
    
    /* Check if allocation fits */
    if (pool->used + aligned_size > pool->size) {
        ERROR_REPORT_ERROR(ERROR_OUT_OF_MEMORY, "Pool exhausted");
        return NULL;
    }
    
    /* Allocate from pool */
    ptr = pool->free_ptr;
    pool->free_ptr += aligned_size;
    pool->used += aligned_size;
    pool->allocations++;
    
    /* Update peak usage */
    if (pool->used > pool->peak_used) {
        pool->peak_used = pool->used;
    }
    
    return ptr;
}

void* memory_pool_calloc(memory_pool_t* pool, size_t count, size_t size) {
    size_t total_size;
    void* ptr;
    
    if (pool == NULL || count == 0 || size == 0) {
        return NULL;
    }
    
    /* Check for overflow */
    if (count > SIZE_MAX / size) {
        ERROR_REPORT_ERROR(ERROR_INVALID_ARGUMENT, "Size overflow");
        return NULL;
    }
    
    total_size = count * size;
    ptr = memory_pool_alloc(pool, total_size);
    
    if (ptr != NULL) {
        memset(ptr, 0, total_size);
    }
    
    return ptr;
}

void memory_pool_free(memory_pool_t* pool, void* ptr) {
    /* Simple pool allocator doesn't support individual frees */
    if (pool != NULL && ptr != NULL) {
        pool->deallocations++;
    }
}

void memory_pool_reset(memory_pool_t* pool) {
    if (pool != NULL) {
        pool->used = 0;
        pool->free_ptr = pool->data;
        /* Keep allocation/deallocation counters for statistics */
    }
}

size_t memory_pool_get_size(const memory_pool_t* pool) {
    return pool ? pool->size : 0;
}

size_t memory_pool_get_used(const memory_pool_t* pool) {
    return pool ? pool->used : 0;
}

size_t memory_pool_get_available(const memory_pool_t* pool) {
    return pool ? pool->size - pool->used : 0;
}

void memory_pool_get_stats(const memory_pool_t* pool, memory_pool_stats_t* stats) {
    if (pool == NULL || stats == NULL) {
        return;
    }
    
    *stats = (memory_pool_stats_t) {
        .total_size = pool->size,
        .used_size = pool->used,
        .peak_used = pool->peak_used,
        .allocations = pool->allocations,
        .deallocations = pool->deallocations,
        .alignment = pool->alignment
    };
}

void* memory_align_ptr(void* ptr, size_t alignment) {
    uintptr_t addr;
    uintptr_t aligned_addr;
    
    if (ptr == NULL || alignment == 0 || alignment == 1) {
        return ptr;
    }
    
    addr = (uintptr_t)ptr;
    aligned_addr = (addr + alignment - 1) & ~(alignment - 1);
    return (void*)aligned_addr;
}

size_t memory_align_size(size_t size, size_t alignment) {
    if (alignment == 0 || alignment == 1) {
        return size;
    }
    
    return (size + alignment - 1) & ~(alignment - 1);
}

bool memory_is_aligned(const void* ptr, size_t alignment) {
    if (ptr == NULL || alignment == 0 || alignment == 1) {
        return true;
    }
    
    return ((uintptr_t)ptr & (alignment - 1)) == 0;
}

#ifdef DEBUG
/* Debug tracking structure */
typedef struct debug_allocation {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    struct debug_allocation* next;
} debug_allocation_t;

static debug_allocation_t* debug_allocations = NULL;

void* memory_pool_alloc_with_debug(memory_pool_t* pool, size_t size, 
                                  const char* file, int line) {
    void* ptr = memory_pool_alloc(pool, size);
    
    if (ptr != NULL) {
        debug_allocation_t* debug_alloc = malloc(sizeof(debug_allocation_t));
        if (debug_alloc != NULL) {
            debug_alloc->ptr = ptr;
            debug_alloc->size = size;
            debug_alloc->file = file;
            debug_alloc->line = line;
            debug_alloc->next = debug_allocations;
            debug_allocations = debug_alloc;
        }
    }
    
    return ptr;
}

void memory_pool_dump_allocations(const memory_pool_t* pool) {
    debug_allocation_t* current;
    int count;
    
    (void)pool; /* Unused parameter */
    
    printf("=== Memory Pool Debug Allocations ===\n");
    current = debug_allocations;
    count = 0;
    
    while (current != NULL) {
        printf("Allocation %d: %p (%zu bytes) at %s:%d\n",
               ++count, current->ptr, current->size, 
               current->file, current->line);
        current = current->next;
    }
    
    printf("Total allocations tracked: %d\n", count);
}
#endif /* DEBUG */
