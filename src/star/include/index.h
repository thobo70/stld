/* src/star/include/index.h */
#ifndef INDEX_H_INCLUDED
#define INDEX_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "star.h"
#include "archive.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file index.h
 * @brief Symbol indexing for STAR archives
 * @details C99 compliant symbol index for fast symbol lookup
 */

/* Index configuration */
#define INDEX_HASH_TABLE_SIZE 1024
#define INDEX_MAX_SYMBOLS 65536
#define INDEX_SYMBOL_NAME_MAX 256

/* Index entry flags */
#define INDEX_FLAG_FUNCTION    0x01
#define INDEX_FLAG_OBJECT      0x02
#define INDEX_FLAG_WEAK        0x04
#define INDEX_FLAG_GLOBAL      0x08
#define INDEX_FLAG_LOCAL       0x10

/* Forward declarations */
typedef struct symbol_index symbol_index_t;
typedef struct symbol_index_entry symbol_index_entry_t;
typedef struct symbol_hash_table symbol_hash_table_t;

/* Symbol index entry */
struct symbol_index_entry {
    char* name;                     /* Symbol name */
    uint32_t name_hash;             /* Cached name hash */
    uint32_t member_index;          /* Member containing symbol */
    uint32_t symbol_value;          /* Symbol value/address */
    uint32_t symbol_size;           /* Symbol size */
    uint8_t symbol_type;            /* Symbol type */
    uint8_t symbol_binding;         /* Symbol binding */
    uint8_t flags;                  /* Index flags */
    uint8_t reserved;               /* Reserved for alignment */
    const char* member_name;        /* Member name (cached) */
    symbol_index_entry_t* next;     /* Hash table chain */
};

/* Symbol index statistics */
typedef struct symbol_index_stats {
    size_t total_symbols;           /* Total number of symbols */
    size_t function_symbols;        /* Number of functions */
    size_t object_symbols;          /* Number of objects */
    size_t global_symbols;          /* Number of global symbols */
    size_t local_symbols;           /* Number of local symbols */
    size_t weak_symbols;            /* Number of weak symbols */
    size_t hash_table_size;         /* Hash table size */
    size_t max_chain_length;        /* Longest hash chain */
    double load_factor;             /* Hash table load factor */
    size_t memory_usage;            /* Memory usage in bytes */
    size_t index_size;              /* Serialized index size */
} symbol_index_stats_t;

/* Symbol index operations */
symbol_index_t* symbol_index_create(size_t hash_table_size);
void symbol_index_destroy(symbol_index_t* index);

/* Symbol management */
int symbol_index_add_symbol(symbol_index_t* index,
                           const char* name,
                           uint32_t member_index,
                           const char* member_name,
                           uint32_t value,
                           uint32_t size,
                           uint8_t type,
                           uint8_t binding);

symbol_index_entry_t* symbol_index_find_symbol(const symbol_index_t* index,
                                              const char* name);

symbol_index_entry_t* symbol_index_find_symbol_hash(const symbol_index_t* index,
                                                   const char* name,
                                                   uint32_t hash);

/* Multiple symbol lookup */
typedef struct symbol_search_result {
    symbol_index_entry_t* symbol;   /* Found symbol */
    double relevance;               /* Search relevance (0.0-1.0) */
} symbol_search_result_t;

int symbol_index_find_symbols_by_pattern(const symbol_index_t* index,
                                        const char* pattern,
                                        symbol_search_result_t* results,
                                        size_t max_results,
                                        size_t* found_count);

int symbol_index_find_symbols_by_member(const symbol_index_t* index,
                                       uint32_t member_index,
                                       symbol_index_entry_t** symbols,
                                       size_t max_symbols,
                                       size_t* found_count);

/* Archive integration */
int symbol_index_build_from_archive(symbol_index_t* index,
                                   const archive_file_t* archive);

int symbol_index_build_from_member(symbol_index_t* index,
                                  const archive_file_t* archive,
                                  const archive_member_t* member,
                                  uint32_t member_index);

/* Serialization */
int symbol_index_serialize(const symbol_index_t* index,
                          uint8_t** data,
                          size_t* size);

int symbol_index_deserialize(symbol_index_t* index,
                            const uint8_t* data,
                            size_t size);

int symbol_index_write_to_file(const symbol_index_t* index,
                              const char* filename);

int symbol_index_read_from_file(symbol_index_t* index,
                               const char* filename);

/* Iteration */
typedef bool (*symbol_index_visitor_t)(const symbol_index_entry_t* symbol, void* user_data);
void symbol_index_foreach(const symbol_index_t* index,
                         symbol_index_visitor_t visitor,
                         void* user_data);

void symbol_index_foreach_in_member(const symbol_index_t* index,
                                   uint32_t member_index,
                                   symbol_index_visitor_t visitor,
                                   void* user_data);

/* Statistics and debugging */
void symbol_index_get_stats(const symbol_index_t* index, symbol_index_stats_t* stats);
void symbol_index_dump(const symbol_index_t* index, FILE* output);
void symbol_index_dump_hash_table(const symbol_index_t* index, FILE* output);

/* Optimization */
int symbol_index_optimize(symbol_index_t* index);
int symbol_index_rehash(symbol_index_t* index, size_t new_size);

/* Validation */
bool symbol_index_validate(const symbol_index_t* index);
int symbol_index_verify_integrity(const symbol_index_t* index);

/* Utility functions */
uint32_t symbol_index_hash_name(const char* name);
bool symbol_index_name_matches_pattern(const char* name, const char* pattern);
const char* symbol_index_type_to_string(uint8_t type);
const char* symbol_index_binding_to_string(uint8_t binding);

/* Pattern matching utilities */
bool symbol_index_pattern_is_wildcard(const char* pattern);
bool symbol_index_pattern_is_regex(const char* pattern);
double symbol_index_calculate_relevance(const char* name, const char* pattern);

/* C99 inline utility functions */
static inline bool symbol_index_entry_is_function(const symbol_index_entry_t* entry) {
    return entry != NULL && (entry->flags & INDEX_FLAG_FUNCTION) != 0;
}

static inline bool symbol_index_entry_is_object(const symbol_index_entry_t* entry) {
    return entry != NULL && (entry->flags & INDEX_FLAG_OBJECT) != 0;
}

static inline bool symbol_index_entry_is_global(const symbol_index_entry_t* entry) {
    return entry != NULL && (entry->flags & INDEX_FLAG_GLOBAL) != 0;
}

static inline bool symbol_index_entry_is_weak(const symbol_index_entry_t* entry) {
    return entry != NULL && (entry->flags & INDEX_FLAG_WEAK) != 0;
}

static inline bool symbol_index_entry_is_local(const symbol_index_entry_t* entry) {
    return entry != NULL && (entry->flags & INDEX_FLAG_LOCAL) != 0;
}

static inline size_t symbol_index_calculate_hash_size(size_t symbol_count) {
    /* Use next power of two, with minimum of 64 */
    size_t size = 64;
    while (size < symbol_count * 2) {
        size *= 2;
    }
    return size;
}

#ifdef __cplusplus
}
#endif

#endif /* INDEX_H_INCLUDED */
