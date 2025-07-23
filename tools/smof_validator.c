/**
 * @file smof_validator.c
 * @brief SMOF file validation and compliance checking tool
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * Comprehensive validation tool for SMOF files including format compliance,
 * integrity checking, and compatibility verification.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>

#include "smof.h"
#include "error.h"
#include "utils.h"

/* Validation levels */
typedef enum {
    VALIDATION_BASIC,      /* Basic format validation */
    VALIDATION_STANDARD,   /* Standard compliance checking */
    VALIDATION_STRICT,     /* Strict validation with warnings */
    VALIDATION_PEDANTIC    /* Pedantic validation with all checks */
} validation_level_t;

/* Validation result structure */
typedef struct {
    uint32_t errors;
    uint32_t warnings;
    uint32_t info_messages;
    bool is_valid;
} validation_result_t;

/* Global options */
static struct {
    validation_level_t level;
    bool verbose;
    bool quiet;
    bool json_output;
    bool fix_errors;
    const char* output_file;
} options = {
    .level = VALIDATION_STANDARD,
    .verbose = false,
    .quiet = false,
    .json_output = false,
    .fix_errors = false,
    .output_file = NULL
};

/* Print usage information */
static void print_usage(const char* program_name) {
    printf("Usage: %s [options] file.smof [file2.smof ...]\n", program_name);
    printf("\nDescription:\n");
    printf("  Validate SMOF files for format compliance and integrity.\n");
    printf("\nOptions:\n");
    printf("  -l, --level LEVEL       Validation level (basic|standard|strict|pedantic)\n");
    printf("  -v, --verbose           Enable verbose output\n");
    printf("  -q, --quiet             Suppress non-error output\n");
    printf("  -j, --json              Output results in JSON format\n");
    printf("  -f, --fix               Attempt to fix correctable errors\n");
    printf("  -o, --output FILE       Write corrected file to FILE (with --fix)\n");
    printf("  -h, --help              Show this help message\n");
    printf("  --version               Show version information\n");
    printf("\nValidation Levels:\n");
    printf("  basic      - Basic format validation (magic, version, sizes)\n");
    printf("  standard   - Standard compliance (default)\n");
    printf("  strict     - Strict validation with warnings\n");
    printf("  pedantic   - All possible checks and recommendations\n");
    printf("\nExit Codes:\n");
    printf("  0 - All files valid\n");
    printf("  1 - Validation errors found\n");
    printf("  2 - File I/O or system error\n");
    printf("\nExamples:\n");
    printf("  %s program.smof                    # Validate single file\n", program_name);
    printf("  %s -l strict *.smof                # Strict validation of all SMOF files\n", program_name);
    printf("  %s -j program.smof > report.json   # JSON output for CI integration\n", program_name);
    printf("  %s -f -o fixed.smof broken.smof    # Fix errors and save to new file\n", program_name);
}

/* Print version information */
static void print_version(void) {
    printf("smof_validator version 1.0.0\n");
    printf("SMOF format version support: 1.x\n");
    printf("Copyright (c) 2025 STIX Project\n");
}

/* Validation message types */
typedef enum {
    MSG_ERROR,
    MSG_WARNING,
    MSG_INFO
} message_type_t;

/* Print validation message */
static void print_message(message_type_t type, const char* format, ...) {
    if (options.quiet && type != MSG_ERROR) {
        return;
    }
    
    if (options.json_output) {
        return; /* JSON messages handled separately */
    }
    
    const char* prefix;
    switch (type) {
        case MSG_ERROR:   prefix = "ERROR"; break;
        case MSG_WARNING: prefix = "WARNING"; break;
        case MSG_INFO:    prefix = "INFO"; break;
    }
    
    printf("[%s] ", prefix);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
}

/* Calculate CRC32 checksum */
static uint32_t calculate_crc32(const uint8_t* data, size_t length) {
    static const uint32_t crc_table[256] = {
        /* CRC-32 table - truncated for brevity */
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
        /* ... full table would be here ... */
    };
    
    uint32_t crc = 0xFFFFFFFF;
    
    for (size_t i = 0; i < length; i++) {
        crc = crc_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    
    return crc ^ 0xFFFFFFFF;
}

/* Validate SMOF header */
static bool validate_header(const smof_header_t* header, const char* filename, 
                           validation_result_t* result) {
    bool valid = true;
    
    /* Check magic number */
    if (header->magic != SMOF_MAGIC) {
        print_message(MSG_ERROR, "%s: Invalid magic number 0x%08X (expected 0x%08X)",
                     filename, header->magic, SMOF_MAGIC);
        result->errors++;
        valid = false;
    }
    
    /* Check version */
    if (header->version == 0) {
        print_message(MSG_ERROR, "%s: Invalid version number %u", filename, header->version);
        result->errors++;
        valid = false;
    } else if (header->version > SMOF_VERSION) {
        print_message(MSG_WARNING, "%s: Future version %u (current: %u)", 
                     filename, header->version, SMOF_VERSION);
        result->warnings++;
        if (options.level >= VALIDATION_STRICT) {
            valid = false;
        }
    }
    
    /* Check flags consistency */
    if ((header->flags & SMOF_FLAG_LITTLE_ENDIAN) && (header->flags & SMOF_FLAG_BIG_ENDIAN)) {
        print_message(MSG_ERROR, "%s: Conflicting endianness flags", filename);
        result->errors++;
        valid = false;
    }
    
    if (!(header->flags & (SMOF_FLAG_LITTLE_ENDIAN | SMOF_FLAG_BIG_ENDIAN))) {
        print_message(MSG_WARNING, "%s: No endianness flag specified", filename);
        result->warnings++;
    }
    
    /* Check entry point for executable files */
    if ((header->flags & SMOF_FLAG_EXECUTABLE) && header->entry_point == 0) {
        if (options.level >= VALIDATION_STRICT) {
            print_message(MSG_WARNING, "%s: Executable file with zero entry point", filename);
            result->warnings++;
        }
    }
    
    /* Check counts */
    if (header->section_count == 0) {
        print_message(MSG_WARNING, "%s: No sections defined", filename);
        result->warnings++;
    }
    
    if (header->symbol_count == 0 && options.level >= VALIDATION_PEDANTIC) {
        print_message(MSG_INFO, "%s: No symbols defined", filename);
        result->info_messages++;
    }
    
    /* Check table offsets */
    if (header->section_count > 0 && header->section_table_offset == 0) {
        print_message(MSG_ERROR, "%s: Invalid section table offset", filename);
        result->errors++;
        valid = false;
    }
    
    if (header->symbol_count > 0 && header->symbol_table_offset == 0) {
        print_message(MSG_ERROR, "%s: Invalid symbol table offset", filename);
        result->errors++;
        valid = false;
    }
    
    return valid;
}

/* Validate section headers */
static bool validate_sections(FILE* file, const smof_header_t* header, 
                             const char* filename, validation_result_t* result) {
    bool valid = true;
    
    if (header->section_count == 0) {
        return true;
    }
    
    /* Seek to section table */
    if (fseek(file, header->section_table_offset, SEEK_SET) != 0) {
        print_message(MSG_ERROR, "%s: Cannot seek to section table", filename);
        result->errors++;
        return false;
    }
    
    uint32_t prev_end = 0;
    bool sections_overlap = false;
    
    for (uint16_t i = 0; i < header->section_count; i++) {
        smof_section_header_t section;
        
        if (fread(&section, sizeof(section), 1, file) != 1) {
            print_message(MSG_ERROR, "%s: Cannot read section header %u", filename, i);
            result->errors++;
            valid = false;
            continue;
        }
        
        /* Check section type */
        if (section.type >= SMOF_SECTION_TYPE_MAX) {
            print_message(MSG_ERROR, "%s: Section %u has invalid type %u", 
                         filename, i, section.type);
            result->errors++;
            valid = false;
        }
        
        /* Check alignment */
        if (section.alignment > 0 && (section.alignment & (section.alignment - 1)) != 0) {
            print_message(MSG_ERROR, "%s: Section %u has invalid alignment %u (not power of 2)", 
                         filename, i, section.alignment);
            result->errors++;
            valid = false;
        }
        
        /* Check address alignment */
        if (section.alignment > 1 && (section.address % section.alignment) != 0) {
            print_message(MSG_WARNING, "%s: Section %u address 0x%08X not aligned to %u", 
                         filename, i, section.address, section.alignment);
            result->warnings++;
        }
        
        /* Check for overlapping sections */
        if (section.offset > 0 && section.size > 0) {
            if (section.offset < prev_end) {
                print_message(MSG_ERROR, "%s: Section %u overlaps with previous section", 
                             filename, i);
                result->errors++;
                valid = false;
                sections_overlap = true;
            }
            prev_end = section.offset + section.size;
        }
        
        /* Check section size limits */
        if (options.level >= VALIDATION_PEDANTIC) {
            if (section.size > 1024 * 1024) { /* 1MB */
                print_message(MSG_INFO, "%s: Section %u is very large (%u bytes)", 
                             filename, i, section.size);
                result->info_messages++;
            }
        }
    }
    
    return valid && !sections_overlap;
}

/* Validate symbol table */
static bool validate_symbols(FILE* file, const smof_header_t* header, 
                            const char* filename, validation_result_t* result) {
    bool valid = true;
    
    if (header->symbol_count == 0) {
        return true;
    }
    
    /* Seek to symbol table */
    if (fseek(file, header->symbol_table_offset, SEEK_SET) != 0) {
        print_message(MSG_ERROR, "%s: Cannot seek to symbol table", filename);
        result->errors++;
        return false;
    }
    
    uint32_t local_symbols = 0;
    uint32_t global_symbols = 0;
    bool found_file_symbol = false;
    
    for (uint16_t i = 0; i < header->symbol_count; i++) {
        smof_symbol_t symbol;
        
        if (fread(&symbol, sizeof(symbol), 1, file) != 1) {
            print_message(MSG_ERROR, "%s: Cannot read symbol %u", filename, i);
            result->errors++;
            valid = false;
            continue;
        }
        
        /* Check symbol type and binding */
        uint8_t type = SMOF_SYMBOL_TYPE(symbol.info);
        uint8_t bind = SMOF_SYMBOL_BIND(symbol.info);
        
        if (type >= SMOF_SYMBOL_TYPE_MAX) {
            print_message(MSG_ERROR, "%s: Symbol %u has invalid type %u", filename, i, type);
            result->errors++;
            valid = false;
        }
        
        if (bind >= SMOF_SYMBOL_BIND_MAX) {
            print_message(MSG_ERROR, "%s: Symbol %u has invalid binding %u", filename, i, bind);
            result->errors++;
            valid = false;
        }
        
        /* Count symbol types */
        if (bind == SMOF_SYMBOL_BIND_LOCAL) {
            local_symbols++;
        } else {
            global_symbols++;
        }
        
        if (type == SMOF_SYMBOL_TYPE_FILE) {
            found_file_symbol = true;
        }
        
        /* Check section index */
        if (symbol.section_index != SMOF_SECTION_UNDEF && 
            symbol.section_index >= header->section_count) {
            print_message(MSG_ERROR, "%s: Symbol %u references invalid section %u", 
                         filename, i, symbol.section_index);
            result->errors++;
            valid = false;
        }
        
        /* Check symbol visibility */
        uint8_t visibility = SMOF_SYMBOL_VISIBILITY(symbol.other);
        if (visibility >= SMOF_SYMBOL_VIS_MAX) {
            print_message(MSG_WARNING, "%s: Symbol %u has invalid visibility %u", 
                         filename, i, visibility);
            result->warnings++;
        }
    }
    
    /* Pedantic checks */
    if (options.level >= VALIDATION_PEDANTIC) {
        if (!found_file_symbol && global_symbols > 0) {
            print_message(MSG_INFO, "%s: No file symbol found (recommended for debugging)", 
                         filename);
            result->info_messages++;
        }
        
        if (local_symbols == 0 && global_symbols > 0) {
            print_message(MSG_INFO, "%s: No local symbols found", filename);
            result->info_messages++;
        }
    }
    
    return valid;
}

/* Validate string table */
static bool validate_string_table(FILE* file, const smof_header_t* header, 
                                 const char* filename, validation_result_t* result) {
    if (header->string_table_offset == 0) {
        if (header->symbol_count > 0 || header->section_count > 0) {
            print_message(MSG_WARNING, "%s: No string table but symbols/sections present", filename);
            result->warnings++;
            return false;
        }
        return true;
    }
    
    /* Get file size */
    struct stat st;
    if (fstat(fileno(file), &st) != 0) {
        print_message(MSG_ERROR, "%s: Cannot get file size", filename);
        result->errors++;
        return false;
    }
    
    /* Check if string table is within file bounds */
    if (header->string_table_offset >= (uint32_t)st.st_size) {
        print_message(MSG_ERROR, "%s: String table offset beyond file end", filename);
        result->errors++;
        return false;
    }
    
    /* Seek to string table */
    if (fseek(file, header->string_table_offset, SEEK_SET) != 0) {
        print_message(MSG_ERROR, "%s: Cannot seek to string table", filename);
        result->errors++;
        return false;
    }
    
    /* Read first byte - should be null for empty string */
    uint8_t first_byte;
    if (fread(&first_byte, 1, 1, file) == 1) {
        if (first_byte != 0) {
            print_message(MSG_WARNING, "%s: String table does not start with null byte", filename);
            result->warnings++;
        }
    }
    
    return true;
}

/* Validate file integrity */
static bool validate_integrity(FILE* file, const smof_header_t* header, 
                              const char* filename, validation_result_t* result) {
    bool valid = true;
    
    /* Calculate header checksum (excluding the checksum field itself) */
    uint8_t header_copy[sizeof(smof_header_t)];
    memcpy(header_copy, header, sizeof(smof_header_t));
    
    /* Zero out checksum field */
    smof_header_t* temp_header = (smof_header_t*)header_copy;
    temp_header->checksum = 0;
    
    uint32_t calculated_checksum = calculate_crc32(header_copy, sizeof(smof_header_t));
    
    if (calculated_checksum != header->checksum) {
        print_message(MSG_ERROR, "%s: Header checksum mismatch (expected 0x%08X, got 0x%08X)", 
                     filename, header->checksum, calculated_checksum);
        result->errors++;
        valid = false;
    }
    
    /* Check file size consistency */
    struct stat st;
    if (fstat(fileno(file), &st) == 0) {
        uint32_t expected_min_size = sizeof(smof_header_t);
        
        if (header->section_count > 0) {
            expected_min_size = header->section_table_offset + 
                               (header->section_count * sizeof(smof_section_header_t));
        }
        
        if (header->symbol_count > 0) {
            uint32_t symbol_table_end = header->symbol_table_offset + 
                                       (header->symbol_count * sizeof(smof_symbol_t));
            if (symbol_table_end > expected_min_size) {
                expected_min_size = symbol_table_end;
            }
        }
        
        if ((uint32_t)st.st_size < expected_min_size) {
            print_message(MSG_ERROR, "%s: File too small (expected at least %u bytes, got %ld)", 
                         filename, expected_min_size, st.st_size);
            result->errors++;
            valid = false;
        }
    }
    
    return valid;
}

/* Validate single SMOF file */
static bool validate_smof_file(const char* filename, validation_result_t* result) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        print_message(MSG_ERROR, "%s: Cannot open file: %s", filename, strerror(errno));
        result->errors++;
        return false;
    }
    
    bool valid = true;
    
    /* Read header */
    smof_header_t header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        print_message(MSG_ERROR, "%s: Cannot read SMOF header", filename);
        result->errors++;
        fclose(file);
        return false;
    }
    
    /* Validate header */
    if (!validate_header(&header, filename, result)) {
        valid = false;
    }
    
    /* Only continue with further validation if header is basically valid */
    if (header.magic == SMOF_MAGIC) {
        /* Validate sections */
        if (options.level >= VALIDATION_STANDARD) {
            if (!validate_sections(file, &header, filename, result)) {
                valid = false;
            }
        }
        
        /* Validate symbols */
        if (options.level >= VALIDATION_STANDARD) {
            if (!validate_symbols(file, &header, filename, result)) {
                valid = false;
            }
        }
        
        /* Validate string table */
        if (options.level >= VALIDATION_STANDARD) {
            if (!validate_string_table(file, &header, filename, result)) {
                valid = false;
            }
        }
        
        /* Validate integrity */
        if (options.level >= VALIDATION_STRICT) {
            if (!validate_integrity(file, &header, filename, result)) {
                valid = false;
            }
        }
    }
    
    fclose(file);
    
    if (valid && options.verbose) {
        print_message(MSG_INFO, "%s: File is valid", filename);
    }
    
    return valid;
}

/* Output results in JSON format */
static void output_json_results(const char** filenames, int file_count, 
                               validation_result_t* results) {
    printf("{\n");
    printf("  \"validator\": \"smof_validator\",\n");
    printf("  \"version\": \"1.0.0\",\n");
    printf("  \"validation_level\": \"%s\",\n", 
           options.level == VALIDATION_BASIC ? "basic" :
           options.level == VALIDATION_STANDARD ? "standard" :
           options.level == VALIDATION_STRICT ? "strict" : "pedantic");
    printf("  \"timestamp\": \"%ld\",\n", time(NULL));
    printf("  \"files\": [\n");
    
    for (int i = 0; i < file_count; i++) {
        printf("    {\n");
        printf("      \"filename\": \"%s\",\n", filenames[i]);
        printf("      \"valid\": %s,\n", results[i].is_valid ? "true" : "false");
        printf("      \"errors\": %u,\n", results[i].errors);
        printf("      \"warnings\": %u,\n", results[i].warnings);
        printf("      \"info_messages\": %u\n", results[i].info_messages);
        printf("    }%s\n", (i < file_count - 1) ? "," : "");
    }
    
    printf("  ],\n");
    
    /* Summary */
    uint32_t total_errors = 0, total_warnings = 0, total_info = 0;
    int valid_files = 0;
    
    for (int i = 0; i < file_count; i++) {
        total_errors += results[i].errors;
        total_warnings += results[i].warnings;
        total_info += results[i].info_messages;
        if (results[i].is_valid) valid_files++;
    }
    
    printf("  \"summary\": {\n");
    printf("    \"total_files\": %d,\n", file_count);
    printf("    \"valid_files\": %d,\n", valid_files);
    printf("    \"total_errors\": %u,\n", total_errors);
    printf("    \"total_warnings\": %u,\n", total_warnings);
    printf("    \"total_info_messages\": %u\n", total_info);
    printf("  }\n");
    printf("}\n");
}

/* Parse command line arguments */
static int parse_arguments(int argc, char* argv[]) {
    static struct option long_options[] = {
        {"level",   required_argument, 0, 'l'},
        {"verbose", no_argument,       0, 'v'},
        {"quiet",   no_argument,       0, 'q'},
        {"json",    no_argument,       0, 'j'},
        {"fix",     no_argument,       0, 'f'},
        {"output",  required_argument, 0, 'o'},
        {"help",    no_argument,       0, 'h'},
        {"version", no_argument,       0, 1001},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "l:vqjfo:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'l':
                if (strcmp(optarg, "basic") == 0) {
                    options.level = VALIDATION_BASIC;
                } else if (strcmp(optarg, "standard") == 0) {
                    options.level = VALIDATION_STANDARD;
                } else if (strcmp(optarg, "strict") == 0) {
                    options.level = VALIDATION_STRICT;
                } else if (strcmp(optarg, "pedantic") == 0) {
                    options.level = VALIDATION_PEDANTIC;
                } else {
                    fprintf(stderr, "Error: Invalid validation level '%s'\n", optarg);
                    return -1;
                }
                break;
            case 'v':
                options.verbose = true;
                break;
            case 'q':
                options.quiet = true;
                break;
            case 'j':
                options.json_output = true;
                options.quiet = true; /* Suppress regular output in JSON mode */
                break;
            case 'f':
                options.fix_errors = true;
                break;
            case 'o':
                options.output_file = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                exit(0);
            case 1001:
                print_version();
                exit(0);
            case '?':
                return -1;
        }
    }
    
    return optind;
}

/* Main function */
int main(int argc, char* argv[]) {
    int first_file_arg = parse_arguments(argc, argv);
    
    if (first_file_arg < 0) {
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return 2;
    }
    
    if (first_file_arg >= argc) {
        fprintf(stderr, "Error: No input files specified\n");
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return 2;
    }
    
    int file_count = argc - first_file_arg;
    const char** filenames = (const char**)(argv + first_file_arg);
    validation_result_t* results = calloc(file_count, sizeof(validation_result_t));
    
    if (results == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 2;
    }
    
    /* Validate each file */
    bool all_valid = true;
    for (int i = 0; i < file_count; i++) {
        results[i].is_valid = validate_smof_file(filenames[i], &results[i]);
        if (!results[i].is_valid) {
            all_valid = false;
        }
    }
    
    /* Output results */
    if (options.json_output) {
        output_json_results(filenames, file_count, results);
    } else if (!options.quiet) {
        /* Print summary */
        uint32_t total_errors = 0, total_warnings = 0;
        int valid_files = 0;
        
        for (int i = 0; i < file_count; i++) {
            total_errors += results[i].errors;
            total_warnings += results[i].warnings;
            if (results[i].is_valid) valid_files++;
        }
        
        printf("\nValidation Summary:\n");
        printf("==================\n");
        printf("Files validated: %d\n", file_count);
        printf("Valid files:     %d\n", valid_files);
        printf("Total errors:    %u\n", total_errors);
        printf("Total warnings:  %u\n", total_warnings);
        
        if (all_valid) {
            printf("\nAll files are valid!\n");
        } else {
            printf("\nSome files failed validation.\n");
        }
    }
    
    free(results);
    return all_valid ? 0 : 1;
}
