# STLD (STIX Linker) Architecture Document

## Executive Summary

STLD is a specialized linker designed for the STIX operating system that processes SMOF (STIX Minimal Object Format) files. It combines multiple object files and libraries into executable programs or shared libraries while maintaining the minimal memory footprint requirements of resource-constrained systems.

## Design Goals

### Primary Objectives
1. **SMOF Format Support**: Complete compatibility with SMOF object files
2. **Memory Efficiency**: Operate within <100KB memory constraints
3. **Fast Linking**: Optimize for quick link times in embedded environments
4. **Position Independence**: Support for PIC/PIE code generation
5. **Debugging Support**: Preserve debugging information when requested

### Performance Targets
- Link time: <1 second for typical embedded applications
- Memory usage: <64KB during linking process
- Output size overhead: <200 bytes per object file

## Architecture Overview

### Core Components

```
STLD Architecture:
┌─────────────────────────────────────┐
│           Command Line Parser        │
├─────────────────────────────────────┤
│           Input File Manager         │
├─────────────────────────────────────┤
│    ┌─────────────┬─────────────┐    │
│    │Symbol Table │Section Mgr  │    │
│    │Manager      │             │    │
├────┼─────────────┼─────────────┼────┤
│    │Relocation   │Memory       │    │
│    │Engine       │Allocator    │    │
├────┼─────────────┼─────────────┼────┤
│    │Debug Info   │Output       │    │
│    │Processor    │Generator    │    │
└────┴─────────────┴─────────────┴────┘
```

## Detailed Component Design

### 1. Symbol Table Manager

The symbol table maintains all symbols from input files and resolves conflicts according to SMOF binding rules.

```c
/* C99 compliant symbol table implementation */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Symbol table entry structure (C99 compliant)
 */
typedef struct stld_symbol {
    uint32_t name_hash;       /* Hash of symbol name for fast lookup */
    uint32_t name_offset;     /* Offset in string table */
    uint32_t value;           /* Symbol value/address */
    uint32_t size;            /* Symbol size in bytes */
    uint16_t section_index;   /* Index of containing section */
    uint8_t  type;            /* Symbol type (function, object, etc.) */
    uint8_t  binding;         /* Symbol binding (local, global, weak) */
    uint8_t  visibility;      /* Symbol visibility */
    uint8_t  flags;           /* Additional flags */
    uint16_t source_file;     /* Index of source file */
} stld_symbol_t;

/* C99 static assertion for structure size */
_Static_assert(sizeof(stld_symbol_t) == 24, "Symbol entry must be 24 bytes");

/**
 * Symbol table implementation using hash table
 */
typedef struct stld_symbol_table {
    stld_symbol_t* symbols;       /* Symbol entries */
    uint32_t* hash_buckets;       /* Hash bucket indices */
    char* string_table;           /* Symbol name strings */
    uint32_t symbol_count;        /* Number of symbols */
    uint32_t symbol_capacity;     /* Allocated symbol slots */
    uint32_t hash_size;           /* Hash table size */
    uint32_t string_table_size;   /* String table size */
    uint32_t string_table_capacity; /* String table capacity */
} stld_symbol_table_t;

/**
 * Add symbol to table with conflict resolution
 * @param table Symbol table to modify
 * @param name Symbol name (null-terminated)
 * @param symbol Symbol data to add
 * @return 0 on success, negative on error
 */
int stld_symbol_table_add(stld_symbol_table_t* table,
                          const char* name,
                          const stld_symbol_t* symbol);

/**
 * Resolve symbol by name
 * @param table Symbol table to search
 * @param name Symbol name to find
 * @return Pointer to symbol or NULL if not found
 */
stld_symbol_t* stld_symbol_table_lookup(const stld_symbol_table_t* table,
                                        const char* name);

/**
 * Handle symbol conflicts (strong vs weak, etc.)
 * @param existing Current symbol in table
 * @param new_symbol New symbol being added
 * @return 0 to keep existing, 1 to replace, negative on error
 */
int stld_symbol_resolve_conflict(const stld_symbol_t* existing,
                                const stld_symbol_t* new_symbol);
```

### 2. Section Manager

Manages sections from all input files, merging compatible sections and organizing memory layout.

```c
/**
 * Section descriptor for linking (C99 compliant)
 */
typedef struct stld_section {
    uint32_t name_offset;     /* Section name in string table */
    uint32_t virtual_addr;    /* Virtual address in output */
    uint32_t file_offset;     /* Offset in output file */
    uint32_t size;            /* Section size */
    uint32_t alignment;       /* Required alignment */
    uint16_t flags;           /* Section flags */
    uint16_t input_count;     /* Number of input sections merged */
    struct stld_input_section* inputs; /* Array of input sections */
} stld_section_t;

/**
 * Input section from object file
 */
typedef struct stld_input_section {
    void* data;               /* Section data */
    uint32_t size;            /* Section size */
    uint32_t offset;          /* Offset within merged section */
    uint16_t file_index;      /* Source file index */
    uint16_t reloc_count;     /* Number of relocations */
    struct smof_relocation* relocs; /* Relocation entries */
} stld_input_section_t;

/**
 * Merge compatible sections from input files
 * @param output Target section to merge into
 * @param inputs Array of input sections
 * @param input_count Number of input sections
 * @return 0 on success, negative on error
 */
int stld_section_merge(stld_section_t* output,
                      const stld_input_section_t* inputs,
                      int input_count);

/**
 * Calculate section layout and addresses
 * @param sections Array of sections to layout
 * @param section_count Number of sections
 * @param base_address Base address for layout
 * @return 0 on success, negative on error
 */
int stld_section_layout(stld_section_t* sections,
                       int section_count,
                       uint32_t base_address);
```

### 3. Relocation Engine

Processes all relocations in input files and applies them to generate correct addresses.

```c
/* C99 compliant relocation processing */
#include "smof.h"

/**
 * Relocation context for processing
 */
typedef struct stld_relocation_context {
    stld_symbol_table_t* symbol_table;
    stld_section_t* sections;
    int section_count;
    uint32_t base_address;
    bool position_independent;    /* C99 bool type */
} stld_relocation_context_t;

/**
 * Apply relocation to target location
 * @param ctx Relocation context
 * @param reloc Relocation to apply
 * @param target_data Target memory location
 * @param target_address Target virtual address
 * @return 0 on success, negative on error
 */
int stld_apply_relocation(const stld_relocation_context_t* ctx,
                         const smof_relocation_t* reloc,
                         uint8_t* target_data,
                         uint32_t target_address);

/**
 * Process all relocations in an input section
 * @param ctx Relocation context
 * @param input Input section with relocations
 * @return 0 on success, negative on error
 */
int stld_process_section_relocations(const stld_relocation_context_t* ctx,
                                   const stld_input_section_t* input);

/**
 * Generate Global Offset Table for PIC code
 * @param ctx Relocation context
 * @param got_section GOT section to populate
 * @return 0 on success, negative on error
 */
int stld_generate_got(const stld_relocation_context_t* ctx,
                     stld_section_t* got_section);
```

### 4. Output Generator

Creates the final output file in various formats including SMOF, binary flat, and others.

```c
/**
 * Output format enumeration (C99 compliant)
 */
typedef enum {
    STLD_OUTPUT_SMOF_EXECUTABLE,    /* SMOF executable format */
    STLD_OUTPUT_SMOF_SHARED_LIB,    /* SMOF shared library */
    STLD_OUTPUT_SMOF_STATIC_LIB,    /* SMOF static library */
    STLD_OUTPUT_SMOF_OBJECT,        /* SMOF relocatable object */
    STLD_OUTPUT_BINARY_FLAT         /* Binary flat format for OS/embedded */
} stld_output_type_t;

/**
 * Output generation options (C99 compliant)
 */
typedef struct stld_output_options {
    const char* output_filename;
    stld_output_type_t output_type; /* Output format type */
    bool position_independent;      /* Generate PIC code */
    bool strip_debug;               /* Remove debug information */
    bool optimize_size;             /* Optimize for size */
    uint32_t entry_point;           /* Entry point address */
    uint32_t load_address;          /* Load address for binary flat */
    uint32_t max_file_size;         /* Maximum output file size */
    bool fill_gaps;                 /* Fill gaps between sections */
    uint8_t fill_value;             /* Value to use for gap filling */
} stld_output_options_t;

/**
 * Generate final output file
 * @param symbols Symbol table
 * @param sections Array of sections
 * @param section_count Number of sections
 * @param options Output options
 * @return 0 on success, negative on error
 */
int stld_generate_output(const stld_symbol_table_t* symbols,
                        const stld_section_t* sections,
                        int section_count,
                        const stld_output_options_t* options);

/**
 * Write SMOF format output
 * @param file Output file handle
 * @param options Output options
 * @param sections Array of sections
 * @param section_count Number of sections
 * @return 0 on success, negative on error
 */
int stld_write_smof_output(FILE* file,
                          const stld_output_options_t* options,
                          const stld_section_t* sections,
                          int section_count);

/**
 * Write binary flat format output
 * @param file Output file handle
 * @param options Output options
 * @param sections Array of sections
 * @param section_count Number of sections
 * @return 0 on success, negative on error
 */
int stld_write_binary_flat(FILE* file,
                          const stld_output_options_t* options,
                          const stld_section_t* sections,
                          int section_count);
```

### Binary Flat Output Format

Binary flat format produces a raw binary image suitable for OS kernels, bootloaders, and embedded systems.

```c
/**
 * Binary flat layout configuration
 */
typedef struct stld_binary_flat_config {
    uint32_t base_address;          /* Base load address */
    uint32_t max_size;              /* Maximum binary size */
    bool align_sections;            /* Align sections to boundaries */
    uint32_t section_alignment;     /* Section alignment (power of 2) */
    bool pad_to_size;               /* Pad output to specific size */
    uint32_t pad_size;              /* Size to pad to */
    uint8_t pad_value;              /* Padding byte value */
    bool generate_map;              /* Generate memory map file */
    const char* map_filename;       /* Map file name */
} stld_binary_flat_config_t;

/**
 * Memory map entry for binary flat output
 */
typedef struct stld_memory_map_entry {
    uint32_t virtual_address;       /* Virtual address */
    uint32_t physical_address;      /* Physical address in file */
    uint32_t size;                  /* Section size */
    uint32_t flags;                 /* Section flags */
    const char* name;               /* Section name */
} stld_memory_map_entry_t;

/**
 * Create binary flat layout from sections
 * @param sections Input sections
 * @param section_count Number of sections
 * @param config Binary flat configuration
 * @param layout Output layout information
 * @return 0 on success, negative on error
 */
int stld_create_binary_flat_layout(const stld_section_t* sections,
                                  int section_count,
                                  const stld_binary_flat_config_t* config,
                                  stld_memory_map_entry_t** layout);

/**
 * Generate memory map file
 * @param filename Map file name
 * @param layout Memory layout entries
 * @param entry_count Number of entries
 * @param config Binary flat configuration
 * @return 0 on success, negative on error
 */
int stld_generate_memory_map(const char* filename,
                            const stld_memory_map_entry_t* layout,
                            int entry_count,
                            const stld_binary_flat_config_t* config);
```
```

## Memory Management Strategy

### Memory Pool Allocation

```c
/* C99 compliant memory pool implementation */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * Memory pool for efficient allocation during linking
 */
typedef struct stld_memory_pool {
    uint8_t* memory;          /* Pool memory */
    size_t size;              /* Total pool size */
    size_t used;              /* Used bytes */
    size_t alignment;         /* Allocation alignment */
} stld_memory_pool_t;

/**
 * Initialize memory pool
 * @param pool Pool to initialize
 * @param size Pool size in bytes
 * @return 0 on success, negative on error
 */
int stld_pool_init(stld_memory_pool_t* pool, size_t size);

/**
 * Allocate from pool
 * @param pool Memory pool
 * @param size Bytes to allocate
 * @return Pointer to allocated memory or NULL on failure
 */
void* stld_pool_alloc(stld_memory_pool_t* pool, size_t size);

/**
 * Reset pool (free all allocations)
 * @param pool Pool to reset
 */
void stld_pool_reset(stld_memory_pool_t* pool);

/**
 * Destroy pool and free memory
 * @param pool Pool to destroy
 */
void stld_pool_destroy(stld_memory_pool_t* pool);
```

### Memory Usage Tracking

```c
/**
 * Track memory usage during linking
 */
typedef struct stld_memory_stats {
    size_t symbol_table_size;
    size_t section_data_size;
    size_t relocation_size;
    size_t string_table_size;
    size_t total_size;
    size_t peak_usage;
} stld_memory_stats_t;

/**
 * Update memory statistics
 * @param stats Statistics structure to update
 */
void stld_memory_stats_update(stld_memory_stats_t* stats);

/**
 * Report memory usage statistics
 * @param stats Statistics to report
 */
void stld_memory_stats_report(const stld_memory_stats_t* stats);
```

## Linking Process Algorithm

### Main Linking Pipeline

```c
/* C99 compliant main linking function */
/**
 * Main linking function
 * @param input_files Array of input file paths
 * @param input_count Number of input files
 * @param output_file Output file path
 * @param options Linking options
 * @return 0 on success, negative on error
 */
int stld_link(const char* const* input_files, int input_count,
              const char* output_file, const stld_link_options_t* options) {
    
    stld_context_t ctx = {0}; /* C99 designated initializer */
    int result = 0;
    
    /* 1. Initialize linking context */
    result = stld_context_init(&ctx, options);
    if (result != 0) goto cleanup;
    
    /* 2. Parse all input files */
    for (int i = 0; i < input_count; i++) {
        result = stld_parse_input_file(&ctx, input_files[i]);
        if (result != 0) goto cleanup;
    }
    
    /* 3. Resolve symbols */
    result = stld_resolve_symbols(&ctx);
    if (result != 0) goto cleanup;
    
    /* 4. Layout sections */
    result = stld_layout_sections(&ctx);
    if (result != 0) goto cleanup;
    
    /* 5. Process relocations */
    result = stld_process_relocations(&ctx);
    if (result != 0) goto cleanup;
    
    /* 6. Generate output */
    result = stld_generate_output_file(&ctx, output_file);
    
cleanup:
    stld_context_cleanup(&ctx);
    return result;
}
```

### Binary Flat Output Implementation

```c
/**
 * Implementation of binary flat output generator
 */
int stld_write_binary_flat(FILE* file,
                          const stld_output_options_t* options,
                          const stld_section_t* sections,
                          int section_count) {
    
    /* Create binary flat configuration */
    stld_binary_flat_config_t config = {
        .base_address = options->load_address,
        .max_size = options->max_file_size,
        .align_sections = true,
        .section_alignment = 4,  /* 4-byte alignment */
        .pad_to_size = false,
        .pad_value = 0x00,
        .generate_map = true,
        .map_filename = NULL  /* Generate from output filename */
    };
    
    /* Calculate memory layout */
    stld_memory_map_entry_t* layout;
    int result = stld_create_binary_flat_layout(sections, section_count, 
                                               &config, &layout);
    if (result != 0) {
        return result;
    }
    
    /* Sort sections by virtual address */
    qsort(layout, section_count, sizeof(stld_memory_map_entry_t),
          stld_compare_by_address);
    
    /* Write binary data */
    uint32_t current_pos = 0;
    for (int i = 0; i < section_count; i++) {
        const stld_memory_map_entry_t* entry = &layout[i];
        const stld_section_t* section = stld_find_section_by_name(
            sections, section_count, entry->name);
        
        if (section == NULL) continue;
        
        /* Skip non-loadable sections for binary flat */
        if (!(section->flags & SMOF_SECTION_LOAD)) {
            continue;
        }
        
        /* Calculate file position */
        uint32_t file_pos = entry->virtual_address - config.base_address;
        
        /* Fill gap if needed */
        if (options->fill_gaps && file_pos > current_pos) {
            result = stld_fill_gap(file, current_pos, file_pos, 
                                  options->fill_value);
            if (result != 0) goto cleanup;
        }
        
        /* Seek to position */
        if (fseek(file, (long)file_pos, SEEK_SET) != 0) {
            result = -STLD_ERROR_IO;
            goto cleanup;
        }
        
        /* Write section data */
        if (fwrite(section->data, 1, section->size, file) != section->size) {
            result = -STLD_ERROR_IO;
            goto cleanup;
        }
        
        current_pos = file_pos + section->size;
    }
    
    /* Generate memory map file */
    if (config.generate_map) {
        char map_filename[256];
        stld_generate_map_filename(options->output_filename, map_filename, 
                                  sizeof(map_filename));
        
        result = stld_generate_memory_map(map_filename, layout, section_count, 
                                         &config);
        if (result != 0) goto cleanup;
    }
    
cleanup:
    free(layout);
    return result;
}

/**
 * Create memory layout for binary flat output
 */
int stld_create_binary_flat_layout(const stld_section_t* sections,
                                  int section_count,
                                  const stld_binary_flat_config_t* config,
                                  stld_memory_map_entry_t** layout) {
    
    *layout = calloc(section_count, sizeof(stld_memory_map_entry_t));
    if (*layout == NULL) {
        return -STLD_ERROR_OUT_OF_MEMORY;
    }
    
    uint32_t current_address = config->base_address;
    int loadable_count = 0;
    
    /* Process sections in order */
    for (int i = 0; i < section_count; i++) {
        const stld_section_t* section = &sections[i];
        
        /* Skip non-loadable sections */
        if (!(section->flags & SMOF_SECTION_LOAD)) {
            continue;
        }
        
        /* Align address if required */
        if (config->align_sections) {
            current_address = ALIGN_UP(current_address, 
                                      config->section_alignment);
        }
        
        /* Check size limits */
        if (config->max_size > 0) {
            uint32_t end_address = current_address + section->size;
            if (end_address - config->base_address > config->max_size) {
                free(*layout);
                *layout = NULL;
                return -STLD_ERROR_SIZE_LIMIT;
            }
        }
        
        /* Create layout entry */
        stld_memory_map_entry_t* entry = &(*layout)[loadable_count];
        entry->virtual_address = current_address;
        entry->physical_address = current_address - config->base_address;
        entry->size = section->size;
        entry->flags = section->flags;
        entry->name = stld_get_section_name(section);
        
        current_address += section->size;
        loadable_count++;
    }
    
    return loadable_count;
}

/**
 * Generate memory map file
 */
int stld_generate_memory_map(const char* filename,
                            const stld_memory_map_entry_t* layout,
                            int entry_count,
                            const stld_binary_flat_config_t* config) {
    
    FILE* map_file = fopen(filename, "w");
    if (map_file == NULL) {
        return -STLD_ERROR_FILE_OPEN;
    }
    
    /* Write map file header */
    fprintf(map_file, "# Memory Map for Binary Flat Output\n");
    fprintf(map_file, "# Generated by STLD v%s\n", STLD_VERSION);
    fprintf(map_file, "# Base Address: 0x%08X\n", config->base_address);
    fprintf(map_file, "#\n");
    fprintf(map_file, "# Virtual Addr   File Offset    Size       Flags  Section\n");
    fprintf(map_file, "# ------------   -----------    --------   -----  -------\n");
    
    /* Write section entries */
    for (int i = 0; i < entry_count; i++) {
        const stld_memory_map_entry_t* entry = &layout[i];
        
        /* Format flags string */
        char flags_str[8] = "-------";
        if (entry->flags & SMOF_SECTION_READ)    flags_str[0] = 'R';
        if (entry->flags & SMOF_SECTION_WRITE)   flags_str[1] = 'W';
        if (entry->flags & SMOF_SECTION_EXEC)    flags_str[2] = 'X';
        if (entry->flags & SMOF_SECTION_LOAD)    flags_str[3] = 'L';
        if (entry->flags & SMOF_SECTION_ALLOC)   flags_str[4] = 'A';
        if (entry->flags & SMOF_SECTION_INIT)    flags_str[5] = 'I';
        if (entry->flags & SMOF_SECTION_DEBUG)   flags_str[6] = 'D';
        
        fprintf(map_file, "  0x%08X     0x%08X   %8u   %s  %s\n",
                entry->virtual_address,
                entry->physical_address,
                entry->size,
                flags_str,
                entry->name);
    }
    
    /* Write summary */
    uint32_t total_size = 0;
    for (int i = 0; i < entry_count; i++) {
        total_size += layout[i].size;
    }
    
    fprintf(map_file, "#\n");
    fprintf(map_file, "# Total Size: %u bytes (0x%X)\n", total_size, total_size);
    
    fclose(map_file);
    return 0;
}
```
```

### Symbol Resolution Algorithm

```c
/**
 * Resolve all undefined symbols
 * @param ctx Linking context
 * @return 0 on success, negative on error
 */
int stld_resolve_symbols(stld_context_t* ctx) {
    int unresolved_count = 0;
    
    /* First pass: collect all defined symbols */
    for (int i = 0; i < (int)ctx->symbol_table.symbol_count; i++) {
        stld_symbol_t* sym = &ctx->symbol_table.symbols[i];
        if (sym->section_index != UINT16_MAX) {  /* Defined symbol */
            stld_symbol_table_define(&ctx->symbol_table, sym);
        }
    }
    
    /* Second pass: resolve undefined symbols */
    for (int i = 0; i < (int)ctx->symbol_table.symbol_count; i++) {
        stld_symbol_t* sym = &ctx->symbol_table.symbols[i];
        if (sym->section_index == UINT16_MAX) {  /* Undefined symbol */
            stld_symbol_t* def = stld_symbol_table_lookup_defined(
                &ctx->symbol_table, 
                stld_symbol_get_name(&ctx->symbol_table, sym)
            );
            if (def != NULL) {
                sym->value = def->value;
                sym->section_index = def->section_index;
            } else {
                if (sym->binding != SMOF_BIND_WEAK) {
                    stld_error("Undefined symbol: %s", 
                              stld_symbol_get_name(&ctx->symbol_table, sym));
                    unresolved_count++;
                }
            }
        }
    }
    
    return (unresolved_count > 0) ? -1 : 0;
}
```

## Testing Strategy

### 1. Unit Testing Framework

```c
// tests/unit/test_symbol_table.c
#include "unity.h"
#include "stld/symbol_table.h"

void setUp(void) {
    // Setup before each test
}

void tearDown(void) {
    // Cleanup after each test
}

void test_symbol_table_creation(void) {
    struct stld_symbol_table table;
    
    int result = stld_symbol_table_init(&table, 64);
    
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_NOT_NULL(table.symbols);
    TEST_ASSERT_NOT_NULL(table.hash_buckets);
    TEST_ASSERT_EQUAL(0, table.symbol_count);
    TEST_ASSERT_EQUAL(64, table.symbol_capacity);
    
    stld_symbol_table_cleanup(&table);
}

void test_symbol_insertion_and_lookup(void) {
    struct stld_symbol_table table;
    struct stld_symbol symbol = {
        .name_offset = 0,
        .value = 0x1000,
        .size = 32,
        .section_index = 1,
        .type = SMOF_SYM_FUNC,
        .binding = SMOF_BIND_GLOBAL
    };
    
    stld_symbol_table_init(&table, 64);
    
    // Add symbol
    int result = stld_symbol_table_add(&table, "test_function", &symbol);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(1, table.symbol_count);
    
    // Lookup symbol
    struct stld_symbol* found = stld_symbol_table_lookup(&table, "test_function");
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL(0x1000, found->value);
    TEST_ASSERT_EQUAL(SMOF_SYM_FUNC, found->type);
    
    stld_symbol_table_cleanup(&table);
}

void test_symbol_conflict_resolution(void) {
    struct stld_symbol_table table;
    struct stld_symbol strong_symbol = {
        .value = 0x1000,
        .binding = SMOF_BIND_GLOBAL
    };
    struct stld_symbol weak_symbol = {
        .value = 0x2000,
        .binding = SMOF_BIND_WEAK
    };
    
    stld_symbol_table_init(&table, 64);
    
    // Add weak symbol first
    stld_symbol_table_add(&table, "conflicted_symbol", &weak_symbol);
    
    // Add strong symbol (should override weak)
    stld_symbol_table_add(&table, "conflicted_symbol", &strong_symbol);
    
    struct stld_symbol* found = stld_symbol_table_lookup(&table, "conflicted_symbol");
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL(0x1000, found->value);  // Strong symbol wins
    TEST_ASSERT_EQUAL(SMOF_BIND_GLOBAL, found->binding);
    
    stld_symbol_table_cleanup(&table);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_symbol_table_creation);
    RUN_TEST(test_symbol_insertion_and_lookup);
    RUN_TEST(test_symbol_conflict_resolution);
    
    return UNITY_END();
}
```

### 2. Integration Testing

```c
// tests/integration/test_full_linking.c
#include "unity.h"
#include "stld/stld.h"

void test_simple_executable_linking(void) {
    // Create test input files
    const char* test_files[] = {
        "tests/fixtures/main.smof",
        "tests/fixtures/lib.smof"
    };
    
    struct stld_link_options options = {
        .output_type = STLD_OUTPUT_EXECUTABLE,
        .entry_point = 0x1000,
        .position_independent = 0
    };
    
    // Perform linking
    int result = stld_link(test_files, 2, "test_output.exe", &options);
    
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_TRUE(file_exists("test_output.exe"));
    
    // Validate output file
    struct smof_header header;
    FILE* f = fopen("test_output.exe", "rb");
    TEST_ASSERT_NOT_NULL(f);
    
    fread(&header, sizeof(header), 1, f);
    TEST_ASSERT_EQUAL_HEX32(0x534D4F46, header.magic);  // 'SMOF'
    TEST_ASSERT_EQUAL(0x1000, header.entry_point);
    
    fclose(f);
    unlink("test_output.exe");
}

void test_shared_library_creation(void) {
    const char* test_files[] = {
        "tests/fixtures/shared_lib.smof"
    };
    
    struct stld_link_options options = {
        .output_type = STLD_OUTPUT_SHARED_LIBRARY,
        .position_independent = 1
    };
    
    int result = stld_link(test_files, 1, "test_lib.so", &options);
    
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_TRUE(file_exists("test_lib.so"));
    
    // Validate shared library flags
    struct smof_header header;
    FILE* f = fopen("test_lib.so", "rb");
    fread(&header, sizeof(header), 1, f);
    
    TEST_ASSERT_TRUE(header.flags & SMOF_FLAG_SHARED_LIB);
    TEST_ASSERT_TRUE(header.flags & SMOF_FLAG_POSITION_INDEP);
    
    fclose(f);
    unlink("test_lib.so");
}
```

### 3. Performance Testing

```c
// tests/performance/benchmark_linking.c
#include <time.h>
#include "stld/stld.h"

void benchmark_linking_speed(void) {
    const char* test_files[100];
    
    // Generate test files
    for (int i = 0; i < 100; i++) {
        char filename[64];
        snprintf(filename, sizeof(filename), "test_obj_%d.smof", i);
        generate_test_smof_file(filename, 1024);  // 1KB each
        test_files[i] = strdup(filename);
    }
    
    struct stld_link_options options = {
        .output_type = STLD_OUTPUT_EXECUTABLE
    };
    
    // Measure linking time
    clock_t start = clock();
    int result = stld_link(test_files, 100, "benchmark_output.exe", &options);
    clock_t end = clock();
    
    double link_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Linked 100 files in %.3f seconds\n", link_time);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_LESS_THAN(1.0, link_time);  // Should complete in <1 second
    
    // Cleanup
    for (int i = 0; i < 100; i++) {
        unlink(test_files[i]);
        free((void*)test_files[i]);
    }
    unlink("benchmark_output.exe");
}

void benchmark_memory_usage(void) {
    struct stld_memory_stats stats = {0};
    
    // Link with memory tracking enabled
    stld_enable_memory_tracking(&stats);
    
    const char* test_files[] = {"tests/fixtures/large_program.smof"};
    struct stld_link_options options = {0};
    
    stld_link(test_files, 1, "memory_test.exe", &options);
    
    printf("Peak memory usage: %zu KB\n", stats.peak_usage / 1024);
    TEST_ASSERT_LESS_THAN(65536, stats.peak_usage);  // <64KB
    
    stld_disable_memory_tracking();
    unlink("memory_test.exe");
}
```

### 4. Error Handling Tests

```c
// tests/unit/test_error_handling.c
void test_invalid_input_file(void) {
    const char* invalid_files[] = {"nonexistent.smof"};
    struct stld_link_options options = {0};
    
    int result = stld_link(invalid_files, 1, "output.exe", &options);
    
    TEST_ASSERT_NOT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(STLD_ERROR_FILE_NOT_FOUND, result);
}

void test_corrupted_smof_file(void) {
    // Create corrupted SMOF file
    FILE* f = fopen("corrupted.smof", "wb");
    uint8_t bad_data[] = {0x45, 0x4C, 0x46, 0x7F};  // ELF magic instead of SMOF
    fwrite(bad_data, 1, sizeof(bad_data), f);
    fclose(f);
    
    const char* files[] = {"corrupted.smof"};
    struct stld_link_options options = {0};
    
    int result = stld_link(files, 1, "output.exe", &options);
    
    TEST_ASSERT_NOT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(STLD_ERROR_INVALID_FORMAT, result);
    
    unlink("corrupted.smof");
}

void test_undefined_symbol_error(void) {
    // Create SMOF file with undefined symbol reference
    create_smof_with_undefined_symbol("undefined_ref.smof", "missing_function");
    
    const char* files[] = {"undefined_ref.smof"};
    struct stld_link_options options = {0};
    
    int result = stld_link(files, 1, "output.exe", &options);
    
    TEST_ASSERT_NOT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(STLD_ERROR_UNDEFINED_SYMBOL, result);
    
    unlink("undefined_ref.smof");
}
```

## Build Integration

### Makefile Configuration

```makefile
# STLD-specific Makefile rules
# C99 compliance and embedded system optimization

# STLD library configuration
STLD_SOURCES := src/stld/symbol_table.c \
                src/stld/section_manager.c \
                src/stld/relocation.c \
                src/stld/output.c \
                src/stld/linker.c

STLD_OBJECTS := $(STLD_SOURCES:src/%.c=$(BUILD_DIR)/%.o)
STLD_DEPS := $(STLD_OBJECTS:.o=.d)

# C99 specific flags for STLD
STLD_CFLAGS := $(CFLAGS) -std=c99 -pedantic \
               -DSTLD_VERSION=\"$(VERSION)\" \
               -DSTLD_MAX_MEMORY=65536

# STLD static library
$(BUILD_DIR)/libstld.a: $(STLD_OBJECTS) $(BUILD_DIR)/libcommon.a
	@echo "Creating STLD library..."
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $(STLD_OBJECTS)
	$(RANLIB) $@

# STLD executable
$(BUILD_DIR)/stld: src/stld/main.c $(BUILD_DIR)/libstld.a
	@echo "Building STLD linker..."
	@mkdir -p $(dir $@)
	$(CC) $(STLD_CFLAGS) $(LDFLAGS) -o $@ $< \
		-L$(BUILD_DIR) -lstld -lcommon

# STLD object files with dependency tracking
$(BUILD_DIR)/stld/%.o: src/stld/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(STLD_CFLAGS) $(CPPFLAGS) -MMD -MP -c -o $@ $<

# Include STLD dependencies
-include $(STLD_DEPS)

# STLD-specific targets
.PHONY: stld-only stld-test stld-install

stld-only: $(BUILD_DIR)/stld

stld-test: $(BUILD_DIR)/stld
	@echo "Testing STLD functionality..."
	$(BUILD_DIR)/stld --version
	$(BUILD_DIR)/stld --help

stld-install: $(BUILD_DIR)/stld $(BUILD_DIR)/libstld.a
	@echo "Installing STLD..."
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include/stld
	install -m 755 $(BUILD_DIR)/stld $(DESTDIR)$(PREFIX)/bin/
	install -m 644 $(BUILD_DIR)/libstld.a $(DESTDIR)$(PREFIX)/lib/
	cp -r src/stld/include/* $(DESTDIR)$(PREFIX)/include/stld/
```

## Documentation

### API Documentation

The STLD API is documented using Doxygen with comprehensive examples and usage patterns. Key documentation includes:

- Public API reference
- Linking process explanation
- Error code definitions
- Performance characteristics
- Memory usage guidelines

### User Manual

#### Command-line Usage

**Basic Linking:**
```bash
# Standard SMOF executable
stld -o program.exe main.smof lib.smof

# Shared library
stld -shared -o library.so obj1.smof obj2.smof

# Static library
stld -static -o library.a obj1.smof obj2.smof
```

**Binary Flat Output for OS Development:**
```bash
# Kernel binary at specific load address
stld -format binary -base 0x100000 -o kernel.bin kernel.smof

# Bootloader with gap filling
stld -format binary -base 0x7C00 -fill-gaps -fill-value 0x90 -o boot.bin boot.smof

# Embedded firmware with size limit
stld -format binary -base 0x08000000 -max-size 0x40000 -o firmware.bin main.smof

# Generate memory map
stld -format binary -base 0x10000000 -map kernel.map -o os.bin *.smof
```

**Advanced Binary Flat Options:**
```bash
# Aligned sections for cache optimization
stld -format binary -base 0x400000 -align 0x1000 -o aligned.bin *.smof

# Pad to specific size (useful for flash programming)
stld -format binary -base 0x0 -pad-size 0x10000 -pad-value 0xFF -o flash.bin *.smof

# Position-independent binary
stld -format binary -pic -base 0x0 -o relocatable.bin *.smof
```

#### Binary Flat Output Features

**Memory Layout Control:**
- **Load Address**: Specify exact memory address where binary will be loaded
- **Section Alignment**: Align sections to cache line or page boundaries
- **Gap Filling**: Fill unused memory regions with specified byte patterns
- **Size Limits**: Enforce maximum binary size for embedded systems

**Memory Map Generation:**
Binary flat output can generate detailed memory maps showing:
- Virtual addresses of all sections
- File offsets for debugging
- Section sizes and flags
- Memory usage summary

**Example Memory Map:**
```
# Memory Map for Binary Flat Output
# Generated by STLD v1.0.0
# Base Address: 0x00100000
#
# Virtual Addr   File Offset    Size       Flags  Section
# ------------   -----------    --------   -----  -------
  0x00100000     0x00000000        4096   RWX-L-  .text
  0x00101000     0x00001000        2048   RW--L-  .data
  0x00101800     0x00001800        1024   RW--L-  .bss
#
# Total Size: 7168 bytes (0x1C00)
```

#### Integration Testing for Binary Flat

```c
// tests/integration/test_binary_flat.c
#include "unity.h"
#include "stld/stld.h"

void test_kernel_binary_generation(void) {
    const char* kernel_files[] = {
        "tests/fixtures/kernel_main.smof",
        "tests/fixtures/kernel_drivers.smof",
        "tests/fixtures/kernel_mm.smof"
    };
    
    stld_output_options_t options = {
        .output_filename = "test_kernel.bin",
        .output_type = STLD_OUTPUT_BINARY_FLAT,
        .load_address = 0x100000,
        .max_file_size = 0x200000,  /* 2MB limit */
        .fill_gaps = true,
        .fill_value = 0x90,         /* NOP instruction */
        .position_independent = false
    };
    
    int result = stld_link(kernel_files, 3, "test_kernel.bin", &options);
    
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_TRUE(file_exists("test_kernel.bin"));
    TEST_ASSERT_TRUE(file_exists("test_kernel.map"));
    
    /* Verify binary format - no headers */
    FILE* f = fopen("test_kernel.bin", "rb");
    TEST_ASSERT_NOT_NULL(f);
    
    /* First bytes should be actual code, not format headers */
    uint8_t first_bytes[4];
    fread(first_bytes, 1, 4, f);
    
    /* Should not start with SMOF magic number */
    uint32_t magic = *(uint32_t*)first_bytes;
    TEST_ASSERT_NOT_EQUAL(0x534D4F46, magic);  /* Not 'SMOF' */
    
    fclose(f);
    unlink("test_kernel.bin");
    unlink("test_kernel.map");
}

void test_bootloader_size_limit(void) {
    const char* boot_files[] = {"tests/fixtures/bootloader.smof"};
    
    stld_output_options_t options = {
        .output_filename = "test_boot.bin",
        .output_type = STLD_OUTPUT_BINARY_FLAT,
        .load_address = 0x7C00,
        .max_file_size = 512,       /* 512 bytes (boot sector) */
        .fill_gaps = true,
        .fill_value = 0x00
    };
    
    int result = stld_link(boot_files, 1, "test_boot.bin", &options);
    
    TEST_ASSERT_EQUAL(0, result);
    
    /* Verify size constraint */
    FILE* f = fopen("test_boot.bin", "rb");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    
    TEST_ASSERT_LESS_OR_EQUAL(512, size);
    
    unlink("test_boot.bin");
}

void test_embedded_firmware_alignment(void) {
    const char* fw_files[] = {
        "tests/fixtures/firmware_main.smof",
        "tests/fixtures/firmware_drivers.smof"
    };
    
    stld_output_options_t options = {
        .output_filename = "test_firmware.bin",
        .output_type = STLD_OUTPUT_BINARY_FLAT,
        .load_address = 0x08000000,  /* ARM Cortex-M flash base */
        .max_file_size = 0x40000,    /* 256KB flash */
        .fill_gaps = true,
        .fill_value = 0xFF           /* Flash erased state */
    };
    
    int result = stld_link(fw_files, 2, "test_firmware.bin", &options);
    
    TEST_ASSERT_EQUAL(0, result);
    
    /* Verify memory map generation */
    TEST_ASSERT_TRUE(file_exists("test_firmware.map"));
    
    /* Parse memory map and verify alignment */
    FILE* map = fopen("test_firmware.map", "r");
    TEST_ASSERT_NOT_NULL(map);
    
    char line[256];
    bool found_text_section = false;
    while (fgets(line, sizeof(line), map)) {
        if (strstr(line, ".text")) {
            uint32_t addr;
            sscanf(line, "  0x%08X", &addr);
            TEST_ASSERT_EQUAL(0, addr % 4);  /* 4-byte aligned */
            found_text_section = true;
            break;
        }
    }
    
    TEST_ASSERT_TRUE(found_text_section);
    fclose(map);
    
    unlink("test_firmware.bin");
    unlink("test_firmware.map");
}
```

#### Error Handling for Binary Flat

```c
void test_binary_flat_size_overflow(void) {
    /* Test size limit enforcement */
    const char* large_files[] = {"tests/fixtures/large_program.smof"};
    
    stld_output_options_t options = {
        .output_filename = "test_overflow.bin",
        .output_type = STLD_OUTPUT_BINARY_FLAT,
        .load_address = 0x1000,
        .max_file_size = 1024,      /* Very small limit */
        .fill_gaps = false
    };
    
    int result = stld_link(large_files, 1, "test_overflow.bin", &options);
    
    TEST_ASSERT_NOT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(STLD_ERROR_SIZE_LIMIT, result);
}

void test_binary_flat_invalid_load_address(void) {
    /* Test invalid load address handling */
    const char* test_files[] = {"tests/fixtures/simple.smof"};
    
    stld_output_options_t options = {
        .output_filename = "test_invalid.bin",
        .output_type = STLD_OUTPUT_BINARY_FLAT,
        .load_address = 0xFFFFFFFF,  /* Invalid address */
        .max_file_size = 0,
        .fill_gaps = false
    };
    
    int result = stld_link(test_files, 1, "test_invalid.bin", &options);
    
    TEST_ASSERT_NOT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(STLD_ERROR_INVALID_ADDRESS, result);
}
```

- Linker script syntax
- Debugging techniques  
- Performance optimization tips

## Conclusion

STLD provides a robust and efficient linking solution specifically designed for the SMOF format and STIX operating system. Its architecture ensures optimal performance in memory-constrained environments while maintaining compatibility with standard Unix linking concepts.

### Key Features

**Multi-Format Output Support:**
- SMOF executable and library formats for STIX OS
- Binary flat format for OS kernels, bootloaders, and embedded systems
- Memory map generation for debugging and analysis
- Flexible section layout and alignment control

**Embedded Systems Optimization:**
- Operates within <64KB memory during linking
- Support for position-independent code generation
- Size limit enforcement for flash memory constraints
- Gap filling and padding for hardware requirements

**OS Development Features:**
- Raw binary output without format headers
- Precise load address control for kernel development
- Section alignment for cache and MMU optimization
- Memory map generation for system debugging

**Quality Assurance:**
The comprehensive testing strategy ensures reliability and correctness across various usage scenarios, including specialized binary flat output testing for embedded and OS development use cases. All functionality is validated through unit tests, integration tests, and real-world usage scenarios.
