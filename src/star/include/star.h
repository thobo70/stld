/* src/star/include/star.h */
#ifndef STAR_H_INCLUDED
#define STAR_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file star.h
 * @brief STIX Archiver (STAR) main API
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * @details
 * This header provides the main API for the STIX archiver. The archiver
 * creates and manages static libraries in STAR format, optimized for
 * embedded systems and resource-constrained environments.
 * 
 * @par C99 Compliance
 * This implementation strictly follows the C99 standard (ISO/IEC 9899:1999)
 * and does not use any compiler-specific extensions.
 * 
 * @par Archive Format
 * STAR archives use a simple, efficient format with optional compression
 * and fast symbol lookup tables for linking performance.
 * 
 * @copyright Copyright (c) 2025 STIX Project
 * @license MIT License
 */

/* STAR version information */
#define STAR_VERSION_MAJOR 1
#define STAR_VERSION_MINOR 0
#define STAR_VERSION_PATCH 0
#define STAR_VERSION_STRING "1.0.0"

/* Archive format constants */
#define STAR_MAGIC 0x53544152U  /* 'STAR' */
#define STAR_VERSION 1U
#define STAR_MEMBER_NAME_MAX 256
#define STAR_MAX_MEMBERS 65535

/* Archive flags */
#define STAR_FLAG_COMPRESSED    0x01
#define STAR_FLAG_INDEXED       0x02
#define STAR_FLAG_SORTED        0x04
#define STAR_FLAG_LITTLE_ENDIAN 0x10
#define STAR_FLAG_BIG_ENDIAN    0x20

/* Member flags */
#define STAR_MEMBER_FLAG_COMPRESSED 0x01
#define STAR_MEMBER_FLAG_EXECUTABLE 0x02
#define STAR_MEMBER_FLAG_READONLY   0x04

/**
 * @brief Archive operation mode
 */
typedef enum {
    STAR_MODE_CREATE = 0,       /**< Create new archive */
    STAR_MODE_EXTRACT = 1,      /**< Extract from archive */
    STAR_MODE_UPDATE = 2,       /**< Update existing archive */
    STAR_MODE_LIST = 3,         /**< List archive contents */
    STAR_MODE_DELETE = 4        /**< Delete members from archive */
} star_mode_t;

/**
 * @brief Compression algorithm
 */
typedef enum {
    STAR_COMPRESS_NONE = 0,     /**< No compression */
    STAR_COMPRESS_LZ4 = 1,      /**< LZ4 compression */
    STAR_COMPRESS_ZLIB = 2,     /**< Zlib compression */
    STAR_COMPRESS_LZMA = 3      /**< LZMA compression */
} star_compression_t;

/**
 * @brief Archive options structure
 */
typedef struct star_options {
    star_compression_t compression; /**< Compression algorithm */
    int compression_level;          /**< Compression level (0-9) */
    bool create_index;              /**< Create symbol index */
    bool sort_members;              /**< Sort members by name */
    bool verbose;                   /**< Enable verbose output */
    bool force_overwrite;           /**< Overwrite existing files */
    size_t max_memory;              /**< Maximum memory usage */
    const char* temp_dir;           /**< Temporary directory */
} star_options_t;

/**
 * @brief Archive member information
 */
typedef struct star_member_info {
    char name[STAR_MEMBER_NAME_MAX]; /**< Member name */
    uint32_t size;                   /**< Uncompressed size */
    uint32_t compressed_size;        /**< Compressed size */
    uint32_t checksum;               /**< CRC32 checksum */
    time_t timestamp;                /**< Modification time */
    uint32_t flags;                  /**< Member flags */
    uint32_t offset;                 /**< Offset in archive */
    star_compression_t compression;  /**< Compression used */
} star_member_info_t;

/**
 * @brief Archive statistics
 */
typedef struct star_stats {
    size_t member_count;             /**< Number of members */
    size_t total_size;               /**< Total uncompressed size */
    size_t compressed_size;          /**< Total compressed size */
    size_t archive_size;             /**< Archive file size */
    double compression_ratio;        /**< Compression ratio */
    size_t symbol_count;             /**< Number of symbols */
    size_t index_size;               /**< Symbol index size */
    double creation_time;            /**< Archive creation time */
} star_stats_t;

/**
 * @brief Archive context (opaque)
 */
typedef struct star_context star_context_t;

/**
 * @brief Progress callback function type
 * 
 * @param operation Current operation description
 * @param progress Progress percentage (0-100)
 * @param user_data User-provided data
 */
typedef void (*star_progress_callback_t)(const char* operation, int progress, void* user_data);

/**
 * @brief Create archive context
 * 
 * @param[in] options Archive options (NULL for defaults)
 * @return Archive context or NULL on failure
 */
star_context_t* star_context_create(const star_options_t* options);

/**
 * @brief Destroy archive context
 * 
 * @param[in] context Archive context to destroy
 */
void star_context_destroy(star_context_t* context);

/**
 * @brief Set progress callback
 * 
 * @param[in] context Archive context
 * @param[in] callback Progress callback function
 * @param[in] user_data User data for callback
 */
void star_set_progress_callback(star_context_t* context,
                               star_progress_callback_t callback,
                               void* user_data);

/**
 * @brief Create new archive
 * 
 * @param[in] context Archive context
 * @param[in] archive_path Path to archive file
 * @param[in] file_list Array of file paths to add
 * @param[in] file_count Number of files to add
 * @return 0 on success, negative error code on failure
 */
int star_create_archive(star_context_t* context,
                       const char* archive_path,
                       const char* const* file_list,
                       size_t file_count);

/**
 * @brief Extract from archive
 * 
 * @param[in] context Archive context
 * @param[in] archive_path Path to archive file
 * @param[in] output_dir Output directory (NULL for current)
 * @param[in] member_list Members to extract (NULL for all)
 * @param[in] member_count Number of members to extract
 * @return 0 on success, negative error code on failure
 */
int star_extract_archive(star_context_t* context,
                        const char* archive_path,
                        const char* output_dir,
                        const char* const* member_list,
                        size_t member_count);

/**
 * @brief Update archive with new/modified files
 * 
 * @param[in] context Archive context
 * @param[in] archive_path Path to archive file
 * @param[in] file_list Array of file paths to update
 * @param[in] file_count Number of files to update
 * @return 0 on success, negative error code on failure
 */
int star_update_archive(star_context_t* context,
                       const char* archive_path,
                       const char* const* file_list,
                       size_t file_count);

/**
 * @brief List archive contents
 * 
 * @param[in] context Archive context
 * @param[in] archive_path Path to archive file
 * @param[out] members Array to store member information
 * @param[in,out] count Input: array size, Output: number of members
 * @return 0 on success, negative error code on failure
 */
int star_list_archive(star_context_t* context,
                     const char* archive_path,
                     star_member_info_t* members,
                     size_t* count);

/**
 * @brief Delete members from archive
 * 
 * @param[in] context Archive context
 * @param[in] archive_path Path to archive file
 * @param[in] member_list Members to delete
 * @param[in] member_count Number of members to delete
 * @return 0 on success, negative error code on failure
 */
int star_delete_members(star_context_t* context,
                       const char* archive_path,
                       const char* const* member_list,
                       size_t member_count);

/**
 * @brief Get archive statistics
 * 
 * @param[in] context Archive context
 * @param[in] archive_path Path to archive file
 * @param[out] stats Statistics structure to fill
 * @return 0 on success, negative error code on failure
 */
int star_get_stats(star_context_t* context,
                  const char* archive_path,
                  star_stats_t* stats);

/**
 * @brief Extract single member to memory
 * 
 * @param[in] context Archive context
 * @param[in] archive_path Path to archive file
 * @param[in] member_name Member to extract
 * @param[out] data Pointer to store extracted data
 * @param[out] size Size of extracted data
 * @return 0 on success, negative error code on failure
 * @note Caller must free the returned data
 */
int star_extract_member_to_memory(star_context_t* context,
                                 const char* archive_path,
                                 const char* member_name,
                                 uint8_t** data,
                                 size_t* size);

/**
 * @brief Add member from memory
 * 
 * @param[in] context Archive context
 * @param[in] archive_path Path to archive file
 * @param[in] member_name Member name
 * @param[in] data Member data
 * @param[in] size Data size
 * @return 0 on success, negative error code on failure
 */
int star_add_member_from_memory(star_context_t* context,
                               const char* archive_path,
                               const char* member_name,
                               const uint8_t* data,
                               size_t size);

/**
 * @brief Get default archive options
 * 
 * @return Default options structure
 */
star_options_t star_get_default_options(void);

/**
 * @brief Validate archive options
 * 
 * @param[in] options Options to validate
 * @return true if valid, false otherwise
 */
bool star_validate_options(const star_options_t* options);

/**
 * @brief Get version information
 * 
 * @return Version string
 */
const char* star_get_version(void);

/* C99 inline utility functions */
static inline bool star_is_compressed(star_compression_t compression) {
    return compression != STAR_COMPRESS_NONE;
}

static inline bool star_member_is_compressed(const star_member_info_t* member) {
    return member != NULL && star_is_compressed(member->compression);
}

static inline double star_calculate_compression_ratio(size_t original, size_t compressed) {
    return original > 0 ? (double)compressed / (double)original : 0.0;
}

static inline bool star_member_is_executable(const star_member_info_t* member) {
    return member != NULL && (member->flags & STAR_MEMBER_FLAG_EXECUTABLE) != 0;
}

#ifdef __cplusplus
}
#endif

#endif /* STAR_H_INCLUDED */
