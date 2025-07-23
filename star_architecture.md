# STAR (STIX Archiver) Architecture Document

## Executive Summary

STAR is a specialized archiver designed for the STIX operating system that creates and manages libraries of SMOF (STIX Minimal Object Format) files. It provides functionality to create, extract, list, and maintain archives while optimizing for the memory constraints and performance requirements of embedded and resource-limited systems.

## Design Goals

### Primary Objectives
1. **SMOF Format Support**: Complete compatibility with SMOF object files
2. **Efficient Storage**: Minimize archive size through compression and optimization
3. **Fast Access**: Quick file extraction and listing operations
4. **Memory Efficiency**: Operate within <100KB memory constraints
5. **Integrity Assurance**: Robust error detection and recovery

### Performance Targets
- Archive creation: <500ms for typical libraries
- File extraction: <100ms per file
- Memory usage: <32KB during operation
- Compression ratio: 20-40% size reduction

## Architecture Overview

### Core Components

```
STAR Architecture:
┌─────────────────────────────────────┐
│         Command Line Parser          │
├─────────────────────────────────────┤
│         Archive Format Handler       │
├─────────────────────────────────────┤
│  ┌───────────────┬─────────────────┐ │
│  │File Index     │Compression      │ │
│  │Manager        │Engine           │ │
├──┼───────────────┼─────────────────┼─┤
│  │Integrity      │Memory           │ │
│  │Checker        │Manager          │ │
├──┼───────────────┼─────────────────┼─┤
│  │I/O Manager    │Error Handler    │ │
└──┴───────────────┴─────────────────┴─┘
```

## Detailed Component Design

### 1. Archive Format

STAR uses a custom archive format optimized for SMOF files.

```c
/* C99 compliant STAR archive format */
#include <stdint.h>
#include <stdbool.h>

/**
 * STAR Archive Header (64 bytes, C99 compliant)
 */
typedef struct star_header {
    uint32_t magic;           /* 0x53544152 ('STAR') */
    uint16_t version;         /* Format version (current: 1) */
    uint16_t flags;           /* Archive flags */
    uint32_t file_count;      /* Number of files in archive */
    uint32_t index_offset;    /* Offset to file index table */
    uint32_t index_size;      /* Size of file index table */
    uint32_t data_offset;     /* Offset to file data section */
    uint32_t data_size;       /* Size of file data section */
    uint32_t string_table_offset; /* Offset to string table */
    uint32_t string_table_size;   /* Size of string table */
    uint64_t timestamp;       /* Archive creation time */
    uint32_t checksum;        /* Archive header checksum */
    uint32_t compression_type; /* Compression algorithm used */
    uint8_t  reserved[12];    /* Reserved for future use */
} star_header_t; /* Total: 64 bytes */

/* C99 static assertion for header size */
_Static_assert(sizeof(star_header_t) == 64, "STAR header must be 64 bytes");

/**
 * Archive flags (C99 preprocessor constants)
 */
#define STAR_FLAG_COMPRESSED    0x0001U  /* Archive uses compression */
#define STAR_FLAG_ENCRYPTED     0x0002U  /* Archive is encrypted */
#define STAR_FLAG_READONLY      0x0004U  /* Archive is read-only */
#define STAR_FLAG_INCREMENTAL   0x0008U  /* Incremental archive */
#define STAR_FLAG_INDEXED       0x0010U  /* Fast index available */
```

### 2. File Index Manager

Manages the file index table for quick access to archived files.

```c
/**
 * File entry in archive index (32 bytes, C99 compliant)
 */
typedef struct star_file_entry {
    uint32_t name_offset;     /* Offset in string table */
    uint32_t data_offset;     /* Offset to file data */
    uint32_t original_size;   /* Original file size */
    uint32_t compressed_size; /* Compressed size (0 if not compressed) */
    uint32_t checksum;        /* File checksum (CRC32) */
    uint64_t timestamp;       /* File modification time */
    uint16_t mode;            /* File permissions */
    uint16_t flags;           /* File-specific flags */
    uint32_t reserved;        /* Reserved for future use */
} star_file_entry_t; /* Total: 32 bytes per file */

/* C99 static assertion for entry size */
_Static_assert(sizeof(star_file_entry_t) == 32, "File entry must be 32 bytes");

/**
 * File index structure (C99 compliant)
 */
typedef struct star_file_index {
    star_file_entry_t* entries;       /* File entries array */
    char* string_table;               /* File name strings */
    uint32_t entry_count;             /* Number of entries */
    uint32_t entry_capacity;          /* Allocated entry slots */
    uint32_t string_table_size;       /* String table size */
    uint32_t string_table_capacity;   /* String table capacity */
    uint32_t* name_hash_table;        /* Hash table for name lookup */
    uint32_t hash_size;               /* Hash table size */
} star_file_index_t;

/**
 * Initialize file index
 * @param index Index structure to initialize
 * @param initial_capacity Initial capacity for entries
 * @return 0 on success, negative on error
 */
int star_index_init(star_file_index_t* index, uint32_t initial_capacity);

/**
 * Add file to index
 * @param index File index
 * @param filename Name of file to add
 * @param entry File entry data
 * @return 0 on success, negative on error
 */
int star_index_add_file(star_file_index_t* index,
                       const char* filename,
                       const star_file_entry_t* entry);

/**
 * Find file in index by name
 * @param index File index to search
 * @param filename Name of file to find
 * @return Pointer to file entry or NULL if not found
 */
star_file_entry_t* star_index_find_file(const star_file_index_t* index,
                                        const char* filename);

/**
 * Callback function for file listing
 */
typedef int (*star_file_callback_t)(const star_file_entry_t* entry, 
                                   const char* filename, 
                                   void* user_data);

/**
 * List all files in index
 * @param index File index
 * @param callback Function to call for each file
 * @param user_data User data to pass to callback
 * @return 0 on success, negative on error
 */
int star_index_list_files(const star_file_index_t* index,
                         star_file_callback_t callback,
                         void* user_data);
```

### 3. Compression Engine

Implements multiple compression algorithms optimized for SMOF files.

```c
/**
 * Compression algorithms
 */
enum star_compression_type {
    STAR_COMPRESS_NONE = 0,
    STAR_COMPRESS_LZ77 = 1,
    STAR_COMPRESS_RLE = 2,
    STAR_COMPRESS_HUFFMAN = 3,
    STAR_COMPRESS_SMOF_OPTIMIZED = 4
};

/**
 * Compression context
 */
struct star_compression_context {
    enum star_compression_type type;
    void* algorithm_context;
    uint8_t* input_buffer;
    uint8_t* output_buffer;
    size_t buffer_size;
    size_t input_size;
    size_t output_size;
};

/**
 * Initialize compression context
 */
int star_compression_init(struct star_compression_context* ctx,
                         enum star_compression_type type,
                         size_t buffer_size);

/**
 * Compress data
 */
int star_compress_data(struct star_compression_context* ctx,
                      const void* input_data, size_t input_size,
                      void* output_data, size_t* output_size);

/**
 * Decompress data
 */
int star_decompress_data(struct star_compression_context* ctx,
                        const void* input_data, size_t input_size,
                        void* output_data, size_t* output_size);

/**
 * SMOF-optimized compression
 */
int star_compress_smof_optimized(const struct smof_header* smof,
                                const void* smof_data, size_t smof_size,
                                void* compressed_data, size_t* compressed_size);
```

### 4. Integrity Checker

Ensures data integrity using checksums and validation.

```c
/**
 * Checksum types
 */
enum star_checksum_type {
    STAR_CHECKSUM_CRC32 = 1,
    STAR_CHECKSUM_CRC64 = 2,
    STAR_CHECKSUM_SHA256 = 3
};

/**
 * Calculate file checksum
 */
uint32_t star_calculate_checksum(const void* data, size_t size,
                                enum star_checksum_type type);

/**
 * Verify file integrity
 */
int star_verify_file_integrity(struct star_file_entry* entry,
                              const void* file_data);

/**
 * Verify archive integrity
 */
int star_verify_archive_integrity(const char* archive_filename);

/**
 * Repair corrupted archive (if possible)
 */
int star_repair_archive(const char* archive_filename,
                       const char* repaired_filename);
```

### 5. Memory Manager

Efficient memory management for archive operations.

```c
/**
 * Memory pool for archive operations
 */
struct star_memory_pool {
    uint8_t* memory;          // Pool memory
    size_t size;              // Total pool size
    size_t used;              // Used bytes
    size_t alignment;         // Allocation alignment
    struct star_allocation* allocations; // Allocation tracking
    uint32_t allocation_count;
};

/**
 * Initialize memory pool
 */
int star_memory_pool_init(struct star_memory_pool* pool, size_t size);

/**
 * Allocate from pool with alignment
 */
void* star_memory_pool_alloc(struct star_memory_pool* pool, 
                            size_t size, size_t alignment);

/**
 * Free specific allocation
 */
void star_memory_pool_free(struct star_memory_pool* pool, void* ptr);

/**
 * Reset entire pool
 */
void star_memory_pool_reset(struct star_memory_pool* pool);

/**
 * Get memory usage statistics
 */
void star_memory_pool_stats(struct star_memory_pool* pool,
                           size_t* total, size_t* used, size_t* free);
```

## Archive Operations Implementation

### 1. Archive Creation

```c
/**
 * Create new archive from multiple files
 */
int star_create_archive(const char* archive_filename,
                       const char** input_files,
                       int file_count,
                       struct star_create_options* options) {
    
    struct star_context ctx = {0};
    int result = 0;
    
    // Initialize context
    result = star_context_init(&ctx, options);
    if (result != 0) goto cleanup;
    
    // Process each input file
    for (int i = 0; i < file_count; i++) {
        result = star_add_file_to_archive(&ctx, input_files[i]);
        if (result != 0) goto cleanup;
    }
    
    // Write archive to disk
    result = star_write_archive(&ctx, archive_filename);
    
cleanup:
    star_context_cleanup(&ctx);
    return result;
}

/**
 * Add single file to archive context
 */
int star_add_file_to_archive(struct star_context* ctx, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return STAR_ERROR_FILE_NOT_FOUND;
    
    // Get file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Read file data
    void* file_data = star_memory_pool_alloc(&ctx->memory_pool, file_size, 1);
    if (!file_data) {
        fclose(file);
        return STAR_ERROR_OUT_OF_MEMORY;
    }
    
    fread(file_data, 1, file_size, file);
    fclose(file);
    
    // Validate SMOF format
    if (!star_validate_smof_file(file_data, file_size)) {
        return STAR_ERROR_INVALID_SMOF;
    }
    
    // Compress if requested
    void* compressed_data = file_data;
    size_t compressed_size = file_size;
    
    if (ctx->options.compression_type != STAR_COMPRESS_NONE) {
        compressed_data = star_memory_pool_alloc(&ctx->memory_pool, file_size * 2, 1);
        if (!compressed_data) return STAR_ERROR_OUT_OF_MEMORY;
        
        int compress_result = star_compress_data(&ctx->compression,
                                               file_data, file_size,
                                               compressed_data, &compressed_size);
        if (compress_result != 0) {
            // Fall back to uncompressed
            compressed_data = file_data;
            compressed_size = file_size;
        }
    }
    
    // Create file entry
    struct star_file_entry entry = {
        .data_offset = ctx->current_data_offset,
        .original_size = file_size,
        .compressed_size = (compressed_data != file_data) ? compressed_size : 0,
        .checksum = star_calculate_checksum(file_data, file_size, STAR_CHECKSUM_CRC32),
        .timestamp = star_get_file_timestamp(filename),
        .mode = star_get_file_mode(filename),
        .flags = 0
    };
    
    // Add to index
    star_index_add_file(&ctx->file_index, filename, &entry);
    
    // Store compressed data
    star_data_buffer_append(&ctx->data_buffer, compressed_data, compressed_size);
    ctx->current_data_offset += compressed_size;
    
    return 0;
}
```

### 2. File Extraction

```c
/**
 * Extract file from archive
 */
int star_extract_file(const char* archive_filename,
                     const char* filename,
                     const char* output_path) {
    
    struct star_archive archive = {0};
    int result = 0;
    
    // Open archive
    result = star_open_archive(&archive, archive_filename);
    if (result != 0) return result;
    
    // Find file in index
    struct star_file_entry* entry = star_index_find_file(&archive.index, filename);
    if (!entry) {
        star_close_archive(&archive);
        return STAR_ERROR_FILE_NOT_FOUND;
    }
    
    // Read compressed data
    void* compressed_data = malloc(entry->compressed_size ? 
                                  entry->compressed_size : entry->original_size);
    if (!compressed_data) {
        star_close_archive(&archive);
        return STAR_ERROR_OUT_OF_MEMORY;
    }
    
    fseek(archive.file, entry->data_offset, SEEK_SET);
    size_t read_size = entry->compressed_size ? entry->compressed_size : entry->original_size;
    fread(compressed_data, 1, read_size, archive.file);
    
    // Decompress if necessary
    void* file_data = compressed_data;
    size_t file_size = read_size;
    
    if (entry->compressed_size > 0) {
        file_data = malloc(entry->original_size);
        if (!file_data) {
            free(compressed_data);
            star_close_archive(&archive);
            return STAR_ERROR_OUT_OF_MEMORY;
        }
        
        file_size = entry->original_size;
        result = star_decompress_data(&archive.compression,
                                    compressed_data, entry->compressed_size,
                                    file_data, &file_size);
        if (result != 0) {
            free(compressed_data);
            free(file_data);
            star_close_archive(&archive);
            return result;
        }
        
        free(compressed_data);
    }
    
    // Verify integrity
    uint32_t checksum = star_calculate_checksum(file_data, file_size, STAR_CHECKSUM_CRC32);
    if (checksum != entry->checksum) {
        free(file_data);
        star_close_archive(&archive);
        return STAR_ERROR_CHECKSUM_MISMATCH;
    }
    
    // Write to output file
    FILE* output_file = fopen(output_path, "wb");
    if (!output_file) {
        free(file_data);
        star_close_archive(&archive);
        return STAR_ERROR_FILE_CREATE;
    }
    
    fwrite(file_data, 1, file_size, output_file);
    fclose(output_file);
    
    // Set file permissions and timestamp
    star_set_file_mode(output_path, entry->mode);
    star_set_file_timestamp(output_path, entry->timestamp);
    
    free(file_data);
    star_close_archive(&archive);
    return 0;
}
```

### 3. Archive Listing

```c
/**
 * List archive contents
 */
int star_list_archive(const char* archive_filename,
                     star_list_callback_t callback,
                     void* user_data) {
    
    struct star_archive archive = {0};
    int result = star_open_archive(&archive, archive_filename);
    if (result != 0) return result;
    
    // Iterate through all files
    for (uint32_t i = 0; i < archive.index.entry_count; i++) {
        struct star_file_entry* entry = &archive.index.entries[i];
        const char* filename = &archive.index.string_table[entry->name_offset];
        
        struct star_file_info info = {
            .filename = filename,
            .original_size = entry->original_size,
            .compressed_size = entry->compressed_size,
            .compression_ratio = entry->compressed_size ? 
                ((float)entry->compressed_size / entry->original_size) * 100.0f : 100.0f,
            .timestamp = entry->timestamp,
            .mode = entry->mode,
            .checksum = entry->checksum
        };
        
        if (!callback(&info, user_data)) {
            break;  // Callback requested stop
        }
    }
    
    star_close_archive(&archive);
    return 0;
}
```

## Testing Strategy

### 1. Unit Tests

```c
// tests/unit/test_star_compression.c
#include "unity.h"
#include "star/compression.h"

void test_lz77_compression_decompression(void) {
    const char* test_data = "Hello, World! This is a test string for compression.";
    size_t input_size = strlen(test_data);
    
    uint8_t compressed[256];
    uint8_t decompressed[256];
    size_t compressed_size = sizeof(compressed);
    size_t decompressed_size = sizeof(decompressed);
    
    struct star_compression_context ctx;
    int result = star_compression_init(&ctx, STAR_COMPRESS_LZ77, 512);
    TEST_ASSERT_EQUAL(0, result);
    
    // Compress
    result = star_compress_data(&ctx, test_data, input_size, 
                               compressed, &compressed_size);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_LESS_THAN(input_size, compressed_size);
    
    // Decompress
    result = star_decompress_data(&ctx, compressed, compressed_size,
                                 decompressed, &decompressed_size);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(input_size, decompressed_size);
    TEST_ASSERT_EQUAL_STRING(test_data, (char*)decompressed);
    
    star_compression_cleanup(&ctx);
}

void test_smof_optimized_compression(void) {
    // Create test SMOF data
    struct smof_header header = {
        .magic = 0x534D4F46,
        .version = 1,
        .section_count = 2,
        .symbol_count = 5
    };
    
    uint8_t test_smof[1024];
    memcpy(test_smof, &header, sizeof(header));
    
    uint8_t compressed[512];
    size_t compressed_size = sizeof(compressed);
    
    int result = star_compress_smof_optimized(&header, test_smof, sizeof(test_smof),
                                            compressed, &compressed_size);
    
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_LESS_THAN(sizeof(test_smof), compressed_size);
}

void test_checksum_calculation(void) {
    const char* test_data = "Test data for checksum";
    uint32_t checksum1 = star_calculate_checksum(test_data, strlen(test_data), 
                                                STAR_CHECKSUM_CRC32);
    uint32_t checksum2 = star_calculate_checksum(test_data, strlen(test_data), 
                                                STAR_CHECKSUM_CRC32);
    
    TEST_ASSERT_EQUAL(checksum1, checksum2);  // Consistent results
    TEST_ASSERT_NOT_EQUAL(0, checksum1);     // Non-zero checksum
    
    // Different data should produce different checksum
    uint32_t checksum3 = star_calculate_checksum("Different data", 13, 
                                                STAR_CHECKSUM_CRC32);
    TEST_ASSERT_NOT_EQUAL(checksum1, checksum3);
}
```

### 2. Integration Tests

```c
// tests/integration/test_star_archive_operations.c
#include "unity.h"
#include "star/star.h"

void test_create_and_extract_archive(void) {
    // Create test SMOF files
    create_test_smof_file("test1.smof", 1024);
    create_test_smof_file("test2.smof", 2048);
    
    const char* input_files[] = {"test1.smof", "test2.smof"};
    struct star_create_options options = {
        .compression_type = STAR_COMPRESS_LZ77,
        .checksum_type = STAR_CHECKSUM_CRC32
    };
    
    // Create archive
    int result = star_create_archive("test.star", input_files, 2, &options);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_TRUE(file_exists("test.star"));
    
    // Verify archive contents
    struct archive_info info;
    result = star_get_archive_info("test.star", &info);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(2, info.file_count);
    
    // Extract files
    result = star_extract_file("test.star", "test1.smof", "extracted1.smof");
    TEST_ASSERT_EQUAL(0, result);
    
    result = star_extract_file("test.star", "test2.smof", "extracted2.smof");
    TEST_ASSERT_EQUAL(0, result);
    
    // Verify extracted files match originals
    TEST_ASSERT_TRUE(files_are_identical("test1.smof", "extracted1.smof"));
    TEST_ASSERT_TRUE(files_are_identical("test2.smof", "extracted2.smof"));
    
    // Cleanup
    unlink("test1.smof");
    unlink("test2.smof");
    unlink("test.star");
    unlink("extracted1.smof");
    unlink("extracted2.smof");
}

void test_archive_corruption_detection(void) {
    // Create valid archive
    create_test_smof_file("test.smof", 1024);
    const char* files[] = {"test.smof"};
    star_create_archive("test.star", files, 1, NULL);
    
    // Corrupt the archive
    corrupt_file_at_offset("test.star", 100, 10);
    
    // Verify corruption is detected
    int result = star_verify_archive_integrity("test.star");
    TEST_ASSERT_NOT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(STAR_ERROR_CHECKSUM_MISMATCH, result);
    
    unlink("test.smof");
    unlink("test.star");
}
```

### 3. Performance Tests

```c
// tests/performance/benchmark_star.c
void benchmark_archive_creation_speed(void) {
    // Create 50 test SMOF files
    char* filenames[50];
    for (int i = 0; i < 50; i++) {
        filenames[i] = malloc(32);
        snprintf(filenames[i], 32, "bench_%02d.smof", i);
        create_test_smof_file(filenames[i], 1024 + (i * 100));
    }
    
    struct star_create_options options = {
        .compression_type = STAR_COMPRESS_LZ77
    };
    
    // Measure creation time
    clock_t start = clock();
    int result = star_create_archive("benchmark.star", 
                                   (const char**)filenames, 50, &options);
    clock_t end = clock();
    
    double creation_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Created archive with 50 files in %.3f seconds\n", creation_time);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_LESS_THAN(0.5, creation_time);  // Should complete in <500ms
    
    // Cleanup
    for (int i = 0; i < 50; i++) {
        unlink(filenames[i]);
        free(filenames[i]);
    }
    unlink("benchmark.star");
}

void benchmark_extraction_speed(void) {
    // Create archive with multiple files
    const char* filenames[] = {"file1.smof", "file2.smof", "file3.smof"};
    for (int i = 0; i < 3; i++) {
        create_test_smof_file(filenames[i], 5120);  // 5KB each
    }
    
    star_create_archive("extract_test.star", filenames, 3, NULL);
    
    // Measure extraction time for each file
    for (int i = 0; i < 3; i++) {
        char output_name[32];
        snprintf(output_name, sizeof(output_name), "extracted_%d.smof", i);
        
        clock_t start = clock();
        int result = star_extract_file("extract_test.star", filenames[i], output_name);
        clock_t end = clock();
        
        double extract_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        
        printf("Extracted %s in %.3f seconds\n", filenames[i], extract_time);
        TEST_ASSERT_EQUAL(0, result);
        TEST_ASSERT_LESS_THAN(0.1, extract_time);  // Should complete in <100ms
        
        unlink(output_name);
    }
    
    // Cleanup
    for (int i = 0; i < 3; i++) {
        unlink(filenames[i]);
    }
    unlink("extract_test.star");
}

void benchmark_memory_usage(void) {
    struct star_memory_stats stats = {0};
    
    // Enable memory tracking
    star_enable_memory_tracking(&stats);
    
    // Create large archive
    const char* files[] = {"large_file.smof"};
    create_test_smof_file("large_file.smof", 32768);  // 32KB
    
    star_create_archive("memory_test.star", files, 1, NULL);
    
    printf("Peak memory usage: %zu KB\n", stats.peak_usage / 1024);
    TEST_ASSERT_LESS_THAN(32768, stats.peak_usage);  // <32KB
    
    star_disable_memory_tracking();
    
    unlink("large_file.smof");
    unlink("memory_test.star");
}
```

### 4. Error Handling Tests

```c
// tests/unit/test_star_error_handling.c
void test_invalid_archive_format(void) {
    // Create file with wrong magic number
    FILE* f = fopen("invalid.star", "wb");
    uint8_t bad_magic[] = {0x45, 0x4C, 0x46, 0x7F};  // ELF magic
    fwrite(bad_magic, 1, sizeof(bad_magic), f);
    fclose(f);
    
    struct star_archive archive;
    int result = star_open_archive(&archive, "invalid.star");
    
    TEST_ASSERT_NOT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(STAR_ERROR_INVALID_FORMAT, result);
    
    unlink("invalid.star");
}

void test_out_of_memory_handling(void) {
    // Simulate memory allocation failure
    star_set_memory_limit(1024);  // Very small limit
    
    // Try to create large archive
    create_test_smof_file("large.smof", 8192);
    const char* files[] = {"large.smof"};
    
    int result = star_create_archive("memory_fail.star", files, 1, NULL);
    
    TEST_ASSERT_NOT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(STAR_ERROR_OUT_OF_MEMORY, result);
    
    star_reset_memory_limit();
    unlink("large.smof");
}

void test_file_not_found_handling(void) {
    int result = star_extract_file("nonexistent.star", "file.smof", "output.smof");
    
    TEST_ASSERT_NOT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(STAR_ERROR_ARCHIVE_NOT_FOUND, result);
}
```

## Build Integration

### Makefile Configuration

```makefile
# STAR-specific Makefile rules
# C99 compliance and embedded system optimization

# STAR library configuration
STAR_SOURCES := src/star/file_index.c \
                src/star/compression.c \
                src/star/integrity.c \
                src/star/memory_manager.c \
                src/star/archive_io.c \
                src/star/archiver.c

STAR_OBJECTS := $(STAR_SOURCES:src/%.c=$(BUILD_DIR)/%.o)
STAR_DEPS := $(STAR_OBJECTS:.o=.d)

# C99 specific flags for STAR
STAR_CFLAGS := $(CFLAGS) -std=c99 -pedantic \
               -DSTAR_VERSION=\"$(VERSION)\" \
               -DSTAR_MAX_MEMORY=32768

# Optional compression support
ifdef HAVE_ZLIB
    STAR_CFLAGS += -DHAVE_ZLIB
    STAR_LDFLAGS += -lz
endif

ifdef HAVE_LZ4
    STAR_CFLAGS += -DHAVE_LZ4
    STAR_LDFLAGS += -llz4
endif

# STAR static library
$(BUILD_DIR)/libstar.a: $(STAR_OBJECTS) $(BUILD_DIR)/libcommon.a
	@echo "Creating STAR library..."
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $(STAR_OBJECTS)
	$(RANLIB) $@

# STAR executable
$(BUILD_DIR)/star: src/star/main.c $(BUILD_DIR)/libstar.a
	@echo "Building STAR archiver..."
	@mkdir -p $(dir $@)
	$(CC) $(STAR_CFLAGS) $(LDFLAGS) $(STAR_LDFLAGS) -o $@ $< \
		-L$(BUILD_DIR) -lstar -lcommon

# STAR object files with dependency tracking
$(BUILD_DIR)/star/%.o: src/star/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(STAR_CFLAGS) $(CPPFLAGS) -MMD -MP -c -o $@ $<

# Include STAR dependencies
-include $(STAR_DEPS)

# STAR-specific targets
.PHONY: star-only star-test star-install

star-only: $(BUILD_DIR)/star

star-test: $(BUILD_DIR)/star
	@echo "Testing STAR functionality..."
	$(BUILD_DIR)/star --version
	$(BUILD_DIR)/star --help
	@echo "Creating test archive..."
	$(BUILD_DIR)/star -c test.star tests/fixtures/*.smof
	@echo "Listing archive contents..."
	$(BUILD_DIR)/star -t test.star
	@echo "Extracting archive..."
	$(BUILD_DIR)/star -x test.star
	@rm -f test.star

star-install: $(BUILD_DIR)/star $(BUILD_DIR)/libstar.a
	@echo "Installing STAR..."
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include/star
	install -m 755 $(BUILD_DIR)/star $(DESTDIR)$(PREFIX)/bin/
	install -m 644 $(BUILD_DIR)/libstar.a $(DESTDIR)$(PREFIX)/lib/
	cp -r src/star/include/* $(DESTDIR)$(PREFIX)/include/star/
```

## Command Line Interface

### Usage Examples

```bash
# Create archive
star -c library.star obj1.smof obj2.smof obj3.smof

# Create with compression
star -c -z library.star *.smof

# Extract all files
star -x library.star

# Extract specific file
star -x library.star obj1.smof

# List archive contents
star -t library.star

# Verify archive integrity
star -v library.star

# Add file to existing archive
star -a library.star new_obj.smof

# Remove file from archive
star -d library.star obj1.smof
```

## Documentation

### API Documentation

Comprehensive API documentation includes:
- Archive creation and manipulation functions
- Compression algorithm details
- Error codes and handling
- Performance characteristics
- Memory usage guidelines

### User Manual

- Command-line usage and options
- Archive format specification
- Compression algorithm comparison
- Troubleshooting guide
- Best practices for library management

## Conclusion

STAR provides a robust and efficient archiving solution specifically designed for SMOF files and the STIX operating system. Its architecture ensures optimal performance in memory-constrained environments while providing essential features like compression, integrity checking, and fast file access. The comprehensive testing strategy ensures reliability and correctness across various usage scenarios and edge cases.
