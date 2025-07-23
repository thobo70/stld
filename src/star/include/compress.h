/* src/star/include/compress.h */
#ifndef COMPRESS_H_INCLUDED
#define COMPRESS_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "star.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file compress.h
 * @brief Compression engine for STAR
 * @details C99 compliant compression/decompression interface
 */

/* Compression result codes */
#define COMPRESS_SUCCESS         0
#define COMPRESS_ERROR_INVALID  -1
#define COMPRESS_ERROR_MEMORY   -2
#define COMPRESS_ERROR_CORRUPT  -3
#define COMPRESS_ERROR_OVERFLOW -4

/* Compression statistics */
typedef struct compression_stats {
    size_t input_size;              /* Original data size */
    size_t output_size;             /* Compressed data size */
    double compression_ratio;       /* Compression ratio */
    double compression_time;        /* Compression time in seconds */
    double decompression_time;      /* Decompression time in seconds */
    size_t memory_used;             /* Peak memory usage */
} compression_stats_t;

/* Compression context (opaque) */
typedef struct compression_context compression_context_t;

/* Compression algorithm interface */
typedef struct compression_algorithm {
    const char* name;               /* Algorithm name */
    star_compression_t type;        /* Algorithm type */
    
    /* Create/destroy context */
    compression_context_t* (*create_context)(int level);
    void (*destroy_context)(compression_context_t* ctx);
    
    /* Compression functions */
    int (*compress)(compression_context_t* ctx,
                   const uint8_t* input, size_t input_size,
                   uint8_t* output, size_t output_size,
                   size_t* compressed_size);
    
    int (*decompress)(compression_context_t* ctx,
                     const uint8_t* input, size_t input_size,
                     uint8_t* output, size_t output_size,
                     size_t* decompressed_size);
    
    /* Utility functions */
    size_t (*get_max_compressed_size)(size_t input_size);
    bool (*validate_compressed_data)(const uint8_t* data, size_t size);
    
    /* Statistics */
    void (*get_stats)(const compression_context_t* ctx, compression_stats_t* stats);
} compression_algorithm_t;

/* Compression engine management */
const compression_algorithm_t* compression_get_algorithm(star_compression_t type);
const compression_algorithm_t* compression_find_algorithm(const char* name);
void compression_list_algorithms(const compression_algorithm_t** algorithms, size_t* count);

/* High-level compression interface */
int compression_compress_data(star_compression_t algorithm,
                            int level,
                            const uint8_t* input,
                            size_t input_size,
                            uint8_t** output,
                            size_t* output_size);

int compression_decompress_data(star_compression_t algorithm,
                              const uint8_t* input,
                              size_t input_size,
                              uint8_t** output,
                              size_t* output_size);

/* Memory management for compressed data */
uint8_t* compression_allocate_buffer(size_t size);
void compression_free_buffer(uint8_t* buffer);

/* Validation and detection */
star_compression_t compression_detect_algorithm(const uint8_t* data, size_t size);
bool compression_validate_data(star_compression_t algorithm,
                              const uint8_t* data,
                              size_t size);

/* Compression level utilities */
int compression_get_default_level(star_compression_t algorithm);
int compression_get_max_level(star_compression_t algorithm);
int compression_normalize_level(star_compression_t algorithm, int level);

/* Algorithm-specific implementations */

/* None (no compression) */
extern const compression_algorithm_t compression_none_algorithm;

/* LZ4 compression */
#ifdef ENABLE_LZ4
extern const compression_algorithm_t compression_lz4_algorithm;
#endif

/* Zlib compression */
#ifdef ENABLE_ZLIB
extern const compression_algorithm_t compression_zlib_algorithm;
#endif

/* LZMA compression */
#ifdef ENABLE_LZMA
extern const compression_algorithm_t compression_lzma_algorithm;
#endif

/* Utility functions */
const char* compression_algorithm_to_string(star_compression_t algorithm);
star_compression_t compression_algorithm_from_string(const char* name);
size_t compression_estimate_compressed_size(star_compression_t algorithm,
                                           size_t input_size,
                                           int level);

/* Performance benchmarking */
typedef struct compression_benchmark {
    star_compression_t algorithm;   /* Algorithm type */
    int level;                      /* Compression level */
    size_t input_size;              /* Test data size */
    double compression_speed;       /* MB/s compression */
    double decompression_speed;     /* MB/s decompression */
    double compression_ratio;       /* Compression ratio */
    size_t memory_usage;            /* Peak memory usage */
} compression_benchmark_t;

int compression_benchmark_algorithm(star_compression_t algorithm,
                                   int level,
                                   const uint8_t* test_data,
                                   size_t test_size,
                                   compression_benchmark_t* result);

/* C99 inline utility functions */
static inline bool compression_is_available(star_compression_t algorithm) {
    return compression_get_algorithm(algorithm) != NULL;
}

static inline bool compression_level_is_valid(star_compression_t algorithm, int level) {
    return level >= 0 && level <= compression_get_max_level(algorithm);
}

static inline double compression_calculate_ratio(size_t original, size_t compressed) {
    return original > 0 ? (double)compressed / (double)original : 0.0;
}

static inline double compression_calculate_savings(size_t original, size_t compressed) {
    return original > 0 ? 1.0 - compression_calculate_ratio(original, compressed) : 0.0;
}

static inline size_t compression_calculate_savings_bytes(size_t original, size_t compressed) {
    return original > compressed ? original - compressed : 0;
}

/* Error handling */
const char* compression_get_error_string(int error_code);

#ifdef __cplusplus
}
#endif

#endif /* COMPRESS_H_INCLUDED */
