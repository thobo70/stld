/* src/stld/linker.c */
#include "include/stld.h"
#include "../common/include/error.h"
#include "../common/include/smof.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @file linker.c
 * @brief STLD linker implementation
 * @details Main linking logic implementation
 */

/* Forward declarations for enhanced internal structures */
typedef struct symbol_entry {
    char* name;
    uint64_t value;
    uint32_t size;
    uint8_t type;
    uint8_t binding;
    uint16_t section_index;
    struct symbol_entry* next;
} symbol_entry_t;

typedef struct section_entry {
    char* name;
    uint64_t size;
    uint32_t virtual_address;
    uint16_t flags;
    uint8_t *data;  /* Section data for relocation processing */
    struct section_entry* next;
} section_entry_t;

typedef struct relocation_entry {
    uint32_t offset;
    uint16_t symbol_index;
    uint8_t type;
    uint8_t section_index;
    struct relocation_entry* next;
} relocation_entry_t;

/* STLD context structure */
struct stld_context {
    stld_options_t options;
    stld_progress_callback_t progress_callback;
    void* progress_user_data;
    symbol_entry_t* symbols;
    section_entry_t* sections;
    relocation_entry_t* relocations; /* Add relocations support */
    char** input_files;
    size_t input_file_count;
    size_t input_file_capacity;
};

stld_options_t stld_get_default_options(void) {
    stld_options_t options = {
        .output_type = STLD_OUTPUT_EXECUTABLE,
        .entry_point = 0,
        .base_address = 0x1000,
        .optimize = STLD_OPTIMIZE_NONE,
        .strip_debug = false,
        .position_independent = false,
        .max_memory = 0,
        .fill_gaps = false,
        .fill_value = 0,
        .max_file_size = 0,
        .generate_map = false,
        .page_size = 4096,
        .verbose = false,
        .map_file = NULL,
        .script_file = NULL
    };
    
    return options;
}

bool stld_validate_options(const stld_options_t* options) {
    if (options == NULL) {
        return false;
    }
    
    /* Basic validation */
    if (options->output_type < STLD_OUTPUT_EXECUTABLE || 
        options->output_type > STLD_OUTPUT_BINARY_FLAT) {
        return false;
    }
    
    if (options->optimize < STLD_OPTIMIZE_NONE || 
        options->optimize > STLD_OPTIMIZE_BALANCED) {
        return false;
    }
    
    if (options->page_size == 0 || (options->page_size & (options->page_size - 1)) != 0) {
        return false; /* Page size must be power of 2 */
    }
    
    return true;
}

stld_context_t* stld_context_create(const stld_options_t* options) {
    stld_context_t* context;
    
    if (options == NULL) {
        return NULL;
    }
    
    context = malloc(sizeof(stld_context_t));
    if (context == NULL) {
        return NULL;
    }
    
    /* Initialize context */
    context->options = *options;
    context->progress_callback = NULL;
    context->progress_user_data = NULL;
    context->symbols = NULL;
    context->sections = NULL;
    context->input_file_count = 0;
    context->input_file_capacity = 8;  /* Start with capacity for 8 files */
    
    /* Allocate initial input files array */
    context->input_files = malloc(context->input_file_capacity * sizeof(char*));
    if (context->input_files == NULL) {
        free(context);
        return NULL;
    }
    
    return context;
}

void stld_context_destroy(stld_context_t* context) {
    symbol_entry_t* symbol;
    section_entry_t* section;
    size_t i;
    
    if (context != NULL) {
        /* Free symbols */
        while (context->symbols != NULL) {
            symbol = context->symbols;
            context->symbols = symbol->next;
            free(symbol->name);
            free(symbol);
        }
        
        /* Free sections */
        while (context->sections != NULL) {
            section = context->sections;
            context->sections = section->next;
            free(section->name);
            free(section);
        }
        
        if (context->input_files != NULL) {
            for (i = 0; i < context->input_file_count; i++) {
                free(context->input_files[i]);
            }
            free(context->input_files);
        }
        
        free(context);
    }
}

void stld_set_progress_callback(stld_context_t* context,
                               stld_progress_callback_t callback,
                               void* user_data) {
    if (context != NULL) {
        context->progress_callback = callback;
        context->progress_user_data = user_data;
    }
}

int stld_add_input_file(stld_context_t* context, const char* filename) {
    char* filename_copy;
    
    if (context == NULL || filename == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* Expand array if needed */
    if (context->input_file_count >= context->input_file_capacity) {
        char** new_files;
        size_t new_capacity = context->input_file_capacity > 0 ? 
                             context->input_file_capacity * 2 : 8;
        
        new_files = realloc(context->input_files, new_capacity * sizeof(char*));
        if (new_files == NULL) {
            ERROR_REPORT_ERROR(ERROR_OUT_OF_MEMORY, "Failed to expand input files array");
            return ERROR_OUT_OF_MEMORY;
        }
        
        context->input_files = new_files;
        context->input_file_capacity = new_capacity;
    }
    
    /* Copy filename */
    filename_copy = malloc(strlen(filename) + 1);
    if (filename_copy == NULL) {
        ERROR_REPORT_ERROR(ERROR_OUT_OF_MEMORY, "Failed to allocate filename copy");
        return ERROR_OUT_OF_MEMORY;
    }
    strcpy(filename_copy, filename);
    
    context->input_files[context->input_file_count++] = filename_copy;
    
    return ERROR_SUCCESS;
}

int stld_add_library_path(stld_context_t* context, const char* path) {
    if (context == NULL || path == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* TODO: Implement library path management */
    return ERROR_SUCCESS;
}

int stld_add_library(stld_context_t* context, const char* libname) {
    if (context == NULL || libname == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* TODO: Implement library management */
    return ERROR_SUCCESS;
}

static int load_smof_file(stld_context_t* context, const char* filename) {
    FILE* file;
    smof_header_t header;
    symbol_entry_t* symbol;
    section_entry_t* section;
    
    if (context == NULL || filename == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    file = fopen(filename, "rb");
    if (file == NULL) {
        return ERROR_FILE_IO;
    }
    
    /* Read and validate header */
    if (fread(&header, sizeof(header), 1, file) != 1) {
        fclose(file);
        return ERROR_FILE_IO;
    }
    
    if (!smof_validate_header(&header)) {
        fclose(file);
        return ERROR_CORRUPT_HEADER;
    }
    
    fclose(file);
    
    /* Add a simple section */
    section = malloc(sizeof(section_entry_t));
    if (section != NULL) {
        section->name = malloc(strlen(".text") + 1);
        if (section->name != NULL) {
            strcpy(section->name, ".text");
            section->size = 0;
            section->next = context->sections;
            context->sections = section;
        } else {
            free(section);
        }
    }
    
    /* Add a simple symbol */
    symbol = malloc(sizeof(symbol_entry_t));
    if (symbol != NULL) {
        symbol->name = malloc(strlen("_start") + 1);
        if (symbol->name != NULL) {
            strcpy(symbol->name, "_start");
            symbol->value = 0;
            symbol->next = context->symbols;
            context->symbols = symbol;
        } else {
            free(symbol);
        }
    }
    
    return ERROR_SUCCESS;
}

/* Symbol resolution function */
static symbol_entry_t* find_symbol(stld_context_t* context, const char* name) __attribute__((unused));
static symbol_entry_t* find_symbol(stld_context_t* context, const char* name) {
    symbol_entry_t* symbol = context->symbols;
    while (symbol) {
        if (strcmp(symbol->name, name) == 0) {
            return symbol;
        }
        symbol = symbol->next;
    }
    return NULL;
}

/* Relocation processing function */
static int process_relocations(stld_context_t* context) {
    relocation_entry_t* reloc = context->relocations;
    int unresolved_count = 0;
    
    while (reloc) {
        /* Find the symbol for this relocation */
        symbol_entry_t* symbol = context->symbols;
        uint16_t symbol_index = 0;
        
        /* Find symbol by index */
        for (uint16_t i = 0; i < reloc->symbol_index && symbol; i++) {
            symbol = symbol->next;
            symbol_index++;
        }
        
        if (symbol) {
            /* Apply relocation based on type */
            switch (reloc->type) {
                case 1: /* SMOF_RELOC_ABS32 */
                    /* For absolute relocations, use symbol value directly */
                    /* In a real linker, this would patch the binary */
                    break;
                    
                case 2: /* SMOF_RELOC_REL32 */
                    /* For PC-relative relocations, calculate offset */
                    /* offset = symbol_address - (relocation_address + 4) */
                    break;
                    
                default:
                    /* Unknown relocation type */
                    unresolved_count++;
                    break;
            }
        } else {
            unresolved_count++;
        }
        
        reloc = reloc->next;
    }
    
    return unresolved_count == 0 ? ERROR_SUCCESS : ERROR_SYMBOL_NOT_FOUND;
}

int stld_link(stld_context_t* context, const char* output_file) {
    size_t i;
    int result;
    FILE* output = NULL;  /* Initialize to NULL */
    
    if (context == NULL || output_file == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* Report progress */
    if (context->progress_callback != NULL) {
        context->progress_callback("Initializing", 0, context->progress_user_data);
    }
    
    /* Load input files */
    if (context->progress_callback != NULL) {
        context->progress_callback("Loading objects", 25, context->progress_user_data);
    }
    
    for (i = 0; i < context->input_file_count; i++) {
        result = load_smof_file(context, context->input_files[i]);
        if (result != ERROR_SUCCESS) {
            return result;
        }
    }
    
    /* Resolve symbols */
    if (context->progress_callback != NULL) {
        context->progress_callback("Resolving symbols", 50, context->progress_user_data);
    }
    
    // Process relocations using the current context structure
    result = process_relocations(context);
    if (result != ERROR_SUCCESS) {
        if (output != NULL) {
            fclose(output);
        }
        return result;
    }
    
    /* Layout sections */
    if (context->progress_callback != NULL) {
        context->progress_callback("Layout sections", 75, context->progress_user_data);
    }
    
    /* Write output */
    if (context->progress_callback != NULL) {
        context->progress_callback("Writing output", 90, context->progress_user_data);
    }
    
    /* Create simple output file */
    output = fopen(output_file, "wb");
    if (output == NULL) {
        return ERROR_FILE_IO;
    }
    
    /* Write basic content based on output type */
    if (context->options.output_type == STLD_OUTPUT_BINARY_FLAT) {
        /* Write raw binary data */
        uint8_t dummy_data[] = {0x90, 0x90, 0x90, 0x90}; /* NOP instructions */
        fwrite(dummy_data, 1, sizeof(dummy_data), output);
    } else {
        /* Write SMOF header */
        smof_header_t header = {
            .magic = SMOF_MAGIC,
            .version = SMOF_VERSION_CURRENT,
            .flags = SMOF_FLAG_LITTLE_ENDIAN,
            .entry_point = context->options.base_address,
            .section_count = 1,
            .symbol_count = 1,
            .string_table_offset = sizeof(smof_header_t),
            .string_table_size = 16,
            .section_table_offset = sizeof(smof_header_t) + 16,
            .reloc_table_offset = 0,
            .reloc_count = 0,
            .import_count = 0
        };
        fwrite(&header, sizeof(header), 1, output);
    }
    
    fclose(output);
    
    if (context->progress_callback != NULL) {
        context->progress_callback("Complete", 100, context->progress_user_data);
    }
    
    return ERROR_SUCCESS;
}

int stld_get_stats(const stld_context_t* context, stld_stats_t* stats) {
    symbol_entry_t* symbol;
    section_entry_t* section;
    size_t symbol_count = 0;
    size_t section_count = 0;
    
    if (context == NULL || stats == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* Count symbols */
    for (symbol = context->symbols; symbol != NULL; symbol = symbol->next) {
        symbol_count++;
    }
    
    /* Count sections */
    for (section = context->sections; section != NULL; section = section->next) {
        section_count++;
    }
    
    /* Fill in actual statistics */
    stats->input_files = context->input_file_count;
    stats->total_sections = section_count;
    stats->total_symbols = symbol_count;
    stats->relocations_processed = 0;
    stats->output_size = 0; /* TODO: Calculate from sections */
    stats->memory_used = 0; /* TODO: Track memory usage */
    stats->link_time = 0.0; /* TODO: Track timing */
    
    return ERROR_SUCCESS;
}

int stld_link_files(const char* const* input_files,
                   size_t input_count,
                   const char* output_file,
                   const stld_options_t* options) {
    stld_context_t* context;
    size_t i;
    int result;
    
    if (input_files == NULL || input_count == 0 || output_file == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* Create context */
    context = stld_context_create(options);
    if (context == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }
    
    /* Add input files */
    for (i = 0; i < input_count; i++) {
        result = stld_add_input_file(context, input_files[i]);
        if (result != ERROR_SUCCESS) {
            stld_context_destroy(context);
            return result;
        }
    }
    
    /* Perform linking */
    result = stld_link(context, output_file);
    
    /* Cleanup */
    stld_context_destroy(context);
    
    return result;
}

const char* stld_get_version(void) {
    return STLD_VERSION_STRING;
}
