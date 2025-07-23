/* src/stld/include/stld.h */
#ifndef STLD_H_INCLUDED
#define STLD_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file stld.h
 * @brief STIX Linker (STLD) main API
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * @details
 * This header provides the main API for the STIX linker. The linker
 * processes SMOF (STIX Minimal Object Format) files and produces
 * executable programs or shared libraries optimized for resource-
 * constrained environments.
 * 
 * @par C99 Compliance
 * This implementation strictly follows the C99 standard (ISO/IEC 9899:1999)
 * and does not use any compiler-specific extensions.
 * 
 * @par Memory Requirements
 * The linker is designed to operate within 64KB of memory during
 * the linking process, making it suitable for embedded systems.
 * 
 * @copyright Copyright (c) 2025 STIX Project
 * @license MIT License
 */

/* STLD version information */
#define STLD_VERSION_MAJOR 1
#define STLD_VERSION_MINOR 0
#define STLD_VERSION_PATCH 0
#define STLD_VERSION_STRING "1.0.0"

/**
 * @brief Linker output type enumeration
 */
typedef enum {
    STLD_OUTPUT_EXECUTABLE,     /**< Executable program */
    STLD_OUTPUT_SHARED_LIBRARY, /**< Shared library */
    STLD_OUTPUT_STATIC_LIBRARY, /**< Static library */
    STLD_OUTPUT_OBJECT,         /**< Relocatable object */
    STLD_OUTPUT_BINARY_FLAT     /**< Binary flat format (OS/embedded) */
} stld_output_type_t;

/**
 * @brief Linker optimization level
 */
typedef enum {
    STLD_OPTIMIZE_NONE = 0,     /**< No optimization */
    STLD_OPTIMIZE_SIZE = 1,     /**< Optimize for size */
    STLD_OPTIMIZE_SPEED = 2,    /**< Optimize for speed */
    STLD_OPTIMIZE_BALANCED = 3  /**< Balance size and speed */
} stld_optimize_level_t;

/**
 * @brief Linker options structure
 * 
 * @details
 * Configuration structure for linker behavior. All fields use
 * C99 standard types and follow embedded systems best practices.
 */
typedef struct stld_options {
    stld_output_type_t output_type;     /**< Type of output to generate */
    uint32_t entry_point;               /**< Entry point address (0 = auto) */
    uint32_t base_address;              /**< Base load address for binary flat */
    stld_optimize_level_t optimize;     /**< Optimization level */
    bool strip_debug;                   /**< Remove debug information */
    bool position_independent;          /**< Generate position independent code */
    size_t max_memory;                  /**< Maximum memory usage (0 = unlimited) */
    bool fill_gaps;                     /**< Fill gaps in binary flat output */
    uint8_t fill_value;                 /**< Byte value for gap filling */
    uint32_t max_file_size;             /**< Maximum output file size */
    bool generate_map;                  /**< Generate memory map file */
    uint32_t page_size;                 /**< Memory page size for alignment */
    bool verbose;                       /**< Enable verbose output */
    const char* map_file;               /**< Custom map file name */
    const char* script_file;            /**< Linker script file */
} stld_options_t;

/**
 * @brief Linker statistics
 */
typedef struct stld_stats {
    size_t input_files;                 /**< Number of input files */
    size_t total_sections;              /**< Total sections processed */
    size_t total_symbols;               /**< Total symbols processed */
    size_t relocations_processed;       /**< Relocations processed */
    size_t output_size;                 /**< Output file size */
    size_t memory_used;                 /**< Peak memory usage */
    double link_time;                   /**< Linking time in seconds */
} stld_stats_t;

/**
 * @brief Linker context (opaque)
 */
typedef struct stld_context stld_context_t;

/**
 * @brief Progress callback function type
 * 
 * @param phase Current linking phase description
 * @param progress Progress percentage (0-100)
 * @param user_data User-provided data
 */
typedef void (*stld_progress_callback_t)(const char* phase, int progress, void* user_data);

/**
 * @brief Create linker context with options
 * 
 * @param[in] options Linker options (NULL for defaults)
 * @return Linker context or NULL on failure
 */
stld_context_t* stld_context_create(const stld_options_t* options);

/**
 * @brief Destroy linker context
 * 
 * @param[in] context Linker context to destroy
 */
void stld_context_destroy(stld_context_t* context);

/**
 * @brief Set progress callback
 * 
 * @param[in] context Linker context
 * @param[in] callback Progress callback function
 * @param[in] user_data User data for callback
 */
void stld_set_progress_callback(stld_context_t* context,
                               stld_progress_callback_t callback,
                               void* user_data);

/**
 * @brief Add input file to linker
 * 
 * @param[in] context Linker context
 * @param[in] filename Input file path
 * @return 0 on success, negative error code on failure
 */
int stld_add_input_file(stld_context_t* context, const char* filename);

/**
 * @brief Add library search path
 * 
 * @param[in] context Linker context
 * @param[in] path Library search path
 * @return 0 on success, negative error code on failure
 */
int stld_add_library_path(stld_context_t* context, const char* path);

/**
 * @brief Add library to link
 * 
 * @param[in] context Linker context
 * @param[in] libname Library name (without lib prefix and .a suffix)
 * @return 0 on success, negative error code on failure
 */
int stld_add_library(stld_context_t* context, const char* libname);

/**
 * @brief Perform linking operation
 * 
 * @param[in] context Linker context
 * @param[in] output_file Output file path
 * @return 0 on success, negative error code on failure
 */
int stld_link(stld_context_t* context, const char* output_file);

/**
 * @brief Get linking statistics
 * 
 * @param[in] context Linker context
 * @param[out] stats Statistics structure to fill
 * @return 0 on success, negative error code on failure
 */
int stld_get_stats(const stld_context_t* context, stld_stats_t* stats);

/**
 * @brief Link multiple object files into output (convenience function)
 * 
 * @param[in] input_files Array of input file paths
 * @param[in] input_count Number of input files
 * @param[in] output_file Output file path
 * @param[in] options Linker options (NULL for defaults)
 * 
 * @return 0 on success, negative error code on failure
 * 
 * @par Thread Safety
 * This function is not thread-safe. Multiple concurrent calls
 * may interfere with each other.
 * 
 * @par Memory Usage
 * Peak memory usage is approximately 2-3 times the total size
 * of input files, plus symbol table overhead.
 * 
 * @note All file paths must be valid and accessible. The function
 *       will create the output file and overwrite if it exists.
 */
int stld_link_files(const char* const* input_files,
                   size_t input_count,
                   const char* output_file,
                   const stld_options_t* options);

/**
 * @brief Get default linker options
 * 
 * @return Default options structure
 */
stld_options_t stld_get_default_options(void);

/**
 * @brief Validate linker options
 * 
 * @param[in] options Options to validate
 * @return true if valid, false otherwise
 */
bool stld_validate_options(const stld_options_t* options);

/**
 * @brief Get version information
 * 
 * @return Version string
 */
const char* stld_get_version(void);

/* C99 inline utility functions */
static inline bool stld_is_executable_output(stld_output_type_t type) {
    return type == STLD_OUTPUT_EXECUTABLE || type == STLD_OUTPUT_BINARY_FLAT;
}

static inline bool stld_is_library_output(stld_output_type_t type) {
    return type == STLD_OUTPUT_SHARED_LIBRARY || type == STLD_OUTPUT_STATIC_LIBRARY;
}

static inline bool stld_needs_relocation(stld_output_type_t type) {
    return type != STLD_OUTPUT_BINARY_FLAT;
}

#ifdef __cplusplus
}
#endif

#endif /* STLD_H_INCLUDED */
