/* src/common/include/utils.h */
#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file utils.h
 * @brief Common utility functions
 * @details C99 compliant utility functions for string manipulation, file I/O, etc.
 */

/* String utilities */
size_t utils_strlen_safe(const char* str, size_t max_len);
char* utils_strdup(const char* str);
int utils_strcmp_safe(const char* s1, const char* s2);
int utils_strncmp_safe(const char* s1, const char* s2, size_t n);
char* utils_strcpy_safe(char* dest, const char* src, size_t dest_size);
char* utils_strcat_safe(char* dest, const char* src, size_t dest_size);

/* Hash functions */
uint32_t utils_hash_string(const char* str);
uint32_t utils_hash_data(const void* data, size_t size);
uint32_t utils_hash_combine(uint32_t hash1, uint32_t hash2);

/* File utilities */
bool utils_file_exists(const char* filename);
size_t utils_file_size(const char* filename);
bool utils_file_is_readable(const char* filename);
bool utils_file_is_writable(const char* filename);
char* utils_get_file_extension(const char* filename);
char* utils_get_basename(const char* path);
char* utils_get_dirname(const char* path);

/* Path utilities */
char* utils_normalize_path(const char* path);
char* utils_join_paths(const char* path1, const char* path2);
bool utils_is_absolute_path(const char* path);

/* Byte order utilities */
uint16_t utils_swap16(uint16_t value);
uint32_t utils_swap32(uint32_t value);
uint64_t utils_swap64(uint64_t value);

bool utils_is_little_endian(void);
bool utils_is_big_endian(void);

uint16_t utils_read_le16(const void* ptr);
uint32_t utils_read_le32(const void* ptr);
uint64_t utils_read_le64(const void* ptr);

uint16_t utils_read_be16(const void* ptr);
uint32_t utils_read_be32(const void* ptr);
uint64_t utils_read_be64(const void* ptr);

void utils_write_le16(void* ptr, uint16_t value);
void utils_write_le32(void* ptr, uint32_t value);
void utils_write_le64(void* ptr, uint64_t value);

void utils_write_be16(void* ptr, uint16_t value);
void utils_write_be32(void* ptr, uint32_t value);
void utils_write_be64(void* ptr, uint64_t value);

/* Math utilities */
uint32_t utils_align_up(uint32_t value, uint32_t alignment);
uint32_t utils_align_down(uint32_t value, uint32_t alignment);
bool utils_is_power_of_two(uint32_t value);
uint32_t utils_next_power_of_two(uint32_t value);

/* Bit manipulation */
int utils_count_set_bits(uint32_t value);
int utils_find_first_set_bit(uint32_t value);
int utils_find_last_set_bit(uint32_t value);

/* Memory utilities */
void utils_memset_secure(void* ptr, int value, size_t size);
int utils_memcmp_constant_time(const void* s1, const void* s2, size_t n);
void* utils_memmem(const void* haystack, size_t haystacklen,
                   const void* needle, size_t needlelen);

/* Time utilities */
typedef struct {
    uint64_t seconds;
    uint32_t nanoseconds;
} utils_timestamp_t;

utils_timestamp_t utils_get_timestamp(void);
double utils_timestamp_diff(const utils_timestamp_t* start, const utils_timestamp_t* end);

/* Buffer management */
typedef struct {
    uint8_t* data;
    size_t size;
    size_t capacity;
} utils_buffer_t;

utils_buffer_t* utils_buffer_create(size_t initial_capacity);
void utils_buffer_destroy(utils_buffer_t* buffer);
bool utils_buffer_resize(utils_buffer_t* buffer, size_t new_capacity);
bool utils_buffer_append(utils_buffer_t* buffer, const void* data, size_t size);
bool utils_buffer_append_byte(utils_buffer_t* buffer, uint8_t byte);
void utils_buffer_clear(utils_buffer_t* buffer);

/* C99 inline utility functions */
static inline uint32_t utils_min_u32(uint32_t a, uint32_t b) {
    return (a < b) ? a : b;
}

static inline uint32_t utils_max_u32(uint32_t a, uint32_t b) {
    return (a > b) ? a : b;
}

static inline size_t utils_min_size(size_t a, size_t b) {
    return (a < b) ? a : b;
}

static inline size_t utils_max_size(size_t a, size_t b) {
    return (a > b) ? a : b;
}

static inline bool utils_is_aligned(uint32_t value, uint32_t alignment) {
    return (value & (alignment - 1)) == 0;
}

/* Debugging utilities */
#ifdef DEBUG
void utils_hexdump(const void* data, size_t size, size_t offset);
void utils_print_timestamp(const char* prefix);
#define UTILS_DEBUG_HEXDUMP(data, size) utils_hexdump(data, size, 0)
#define UTILS_DEBUG_TIMESTAMP(prefix) utils_print_timestamp(prefix)
#else
#define UTILS_DEBUG_HEXDUMP(data, size) ((void)0)
#define UTILS_DEBUG_TIMESTAMP(prefix) ((void)0)
#endif

/* Version string utilities */
typedef struct {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
    const char* suffix;
} utils_version_t;

bool utils_parse_version(const char* version_str, utils_version_t* version);
int utils_compare_versions(const utils_version_t* v1, const utils_version_t* v2);
char* utils_format_version(const utils_version_t* version);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H_INCLUDED */
