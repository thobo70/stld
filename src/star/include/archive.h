/* src/star/include/archive.h */
#ifndef ARCHIVE_H_INCLUDED
#define ARCHIVE_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include "star.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file archive.h
 * @brief Archive format handling for STAR
 * @details C99 compliant archive file format implementation
 */

/* Archive file format constants */
#define STAR_HEADER_SIZE 64
#define STAR_MEMBER_HEADER_SIZE 128
#define STAR_CHECKSUM_SIZE 4

/* Archive header structure */
typedef struct star_header {
    uint32_t magic;                 /* Archive magic 'STAR' */
    uint16_t version;               /* Format version */
    uint16_t flags;                 /* Archive flags */
    uint32_t member_count;          /* Number of members */
    uint32_t index_offset;          /* Symbol index offset */
    uint32_t index_size;            /* Symbol index size */
    uint32_t member_table_offset;   /* Member table offset */
    uint32_t string_table_offset;   /* String table offset */
    uint32_t string_table_size;     /* String table size */
    uint32_t creation_time;         /* Archive creation time */
    uint32_t checksum;              /* Header checksum */
    uint8_t reserved[24];           /* Reserved for future use */
} star_header_t;

/* Member header structure */
typedef struct star_member_header {
    uint32_t name_offset;           /* Name in string table */
    uint32_t size;                  /* Uncompressed size */
    uint32_t compressed_size;       /* Compressed size */
    uint32_t data_offset;           /* Data offset in archive */
    uint32_t checksum;              /* Member checksum */
    uint32_t timestamp;             /* Modification timestamp */
    uint16_t flags;                 /* Member flags */
    uint8_t compression;            /* Compression algorithm */
    uint8_t reserved1;              /* Reserved */
    uint8_t reserved2[100];         /* Reserved for future use */
} star_member_header_t;

/* Symbol index entry */
typedef struct star_symbol_entry {
    uint32_t name_offset;           /* Symbol name offset */
    uint32_t member_index;          /* Member containing symbol */
    uint32_t symbol_value;          /* Symbol value */
    uint8_t symbol_type;            /* Symbol type */
    uint8_t symbol_binding;         /* Symbol binding */
    uint16_t reserved;              /* Reserved */
} star_symbol_entry_t;

/* Forward declarations */
typedef struct archive_file archive_file_t;
typedef struct archive_member archive_member_t;

/* Archive file handle */
struct archive_file {
    FILE* file;                     /* File handle */
    star_header_t header;           /* Archive header */
    archive_member_t* members;      /* Member array */
    char* string_table;             /* String table */
    star_symbol_entry_t* symbols;   /* Symbol index */
    bool is_open;                   /* File is open */
    bool is_writable;               /* File is writable */
    const char* filename;           /* Archive filename */
};

/* Archive member */
struct archive_member {
    star_member_header_t header;    /* Member header */
    char* name;                     /* Member name */
    uint8_t* data;                  /* Member data (if loaded) */
    bool data_loaded;               /* Data is loaded in memory */
    uint32_t index;                 /* Member index */
};

/* C99 static assertions for structure sizes */
_Static_assert(sizeof(star_header_t) == 64, "STAR header must be 64 bytes");
_Static_assert(sizeof(star_member_header_t) == 128, "STAR member header must be 128 bytes");
_Static_assert(sizeof(star_symbol_entry_t) == 16, "STAR symbol entry must be 16 bytes");

/* Archive file operations */
archive_file_t* archive_open(const char* filename, const char* mode);
void archive_close(archive_file_t* archive);
bool archive_is_valid(const archive_file_t* archive);

/* Archive creation */
archive_file_t* archive_create(const char* filename, const star_options_t* options);
int archive_write_header(archive_file_t* archive);
int archive_finalize(archive_file_t* archive);

/* Member management */
int archive_add_member(archive_file_t* archive,
                      const char* name,
                      const uint8_t* data,
                      size_t size,
                      time_t timestamp,
                      uint32_t flags);

int archive_add_member_from_file(archive_file_t* archive,
                                const char* member_name,
                                const char* file_path);

archive_member_t* archive_find_member(const archive_file_t* archive, const char* name);
archive_member_t* archive_get_member(const archive_file_t* archive, uint32_t index);

/* Archive loading */
int archive_load_members(archive_file_t* archive);

int archive_extract_member(const archive_file_t* archive,
                          const archive_member_t* member,
                          const char* output_path);

int archive_extract_member_to_memory(const archive_file_t* archive,
                                    const archive_member_t* member,
                                    uint8_t** data,
                                    size_t* size);

int archive_delete_member(archive_file_t* archive, const char* name);

/* Symbol index management */
int archive_build_symbol_index(archive_file_t* archive);
int archive_add_symbol(archive_file_t* archive,
                      const char* name,
                      uint32_t member_index,
                      uint32_t value,
                      uint8_t type,
                      uint8_t binding);

star_symbol_entry_t* archive_find_symbol(const archive_file_t* archive, const char* name);

/* String table management */
int archive_add_string(archive_file_t* archive, const char* str, uint32_t* offset);
const char* archive_get_string(const archive_file_t* archive, uint32_t offset);

/* Validation and integrity */
bool archive_validate_header(const star_header_t* header);
uint32_t archive_calculate_checksum(const void* data, size_t size);
bool archive_verify_member_checksum(const archive_member_t* member);
int archive_verify_integrity(const archive_file_t* archive);

/* Statistics */
void archive_get_member_info(const archive_member_t* member, star_member_info_t* info);
size_t archive_calculate_total_size(const archive_file_t* archive);
size_t archive_calculate_compressed_size(const archive_file_t* archive);

/* Iteration */
typedef bool (*archive_member_visitor_t)(const archive_member_t* member, void* user_data);
void archive_foreach_member(const archive_file_t* archive,
                           archive_member_visitor_t visitor,
                           void* user_data);

typedef bool (*archive_symbol_visitor_t)(const star_symbol_entry_t* symbol,
                                        const char* name,
                                        void* user_data);
void archive_foreach_symbol(const archive_file_t* archive,
                           archive_symbol_visitor_t visitor,
                           void* user_data);

/* Utility functions */
const char* archive_compression_to_string(star_compression_t compression);
star_compression_t archive_compression_from_string(const char* str);
const char* archive_get_error_string(int error_code);

/* Debugging */
void archive_dump_header(const archive_file_t* archive, FILE* output);
void archive_dump_members(const archive_file_t* archive, FILE* output);
void archive_dump_symbols(const archive_file_t* archive, FILE* output);

/* C99 inline utility functions */
static inline bool archive_header_is_valid(const star_header_t* header) {
    return header != NULL && 
           header->magic == STAR_MAGIC && 
           header->version == STAR_VERSION;
}

static inline bool archive_is_compressed(const archive_file_t* archive) {
    return archive != NULL && (archive->header.flags & STAR_FLAG_COMPRESSED) != 0;
}

static inline bool archive_has_index(const archive_file_t* archive) {
    return archive != NULL && (archive->header.flags & STAR_FLAG_INDEXED) != 0;
}

static inline bool archive_is_sorted(const archive_file_t* archive) {
    return archive != NULL && (archive->header.flags & STAR_FLAG_SORTED) != 0;
}

static inline bool archive_member_is_compressed(const archive_member_t* member) {
    return member != NULL && (member->header.flags & STAR_MEMBER_FLAG_COMPRESSED) != 0;
}

static inline size_t archive_member_get_size(const archive_member_t* member) {
    return member ? member->header.size : 0;
}

static inline time_t archive_member_get_timestamp(const archive_member_t* member) {
    return member ? (time_t)member->header.timestamp : 0;
}

#ifdef __cplusplus
}
#endif

#endif /* ARCHIVE_H_INCLUDED */
