/* tools/smof_dump.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "smof.h"
#include "utils.h"
#include "error.h"

/**
 * @file smof_dump.c
 * @brief SMOF file analysis tool
 * @details Utility to analyze and dump SMOF file contents
 */

static void print_usage(const char* program_name) {
    printf("Usage: %s [options] file.smof\n", program_name);
    printf("\nOptions:\n");
    printf("  -h, --header       Show header information only\n");
    printf("  -s, --sections     Show section table\n");
    printf("  -y, --symbols      Show symbol table\n");
    printf("  -r, --relocations  Show relocations\n");
    printf("  -x, --hex-dump     Show hex dump of sections\n");
    printf("  -v, --verbose      Enable verbose output\n");
    printf("  --help             Show this help message\n");
    printf("  --version          Show version information\n");
}

static void print_version(void) {
    printf("smof_dump version 1.0.0\n");
    printf("Copyright (c) 2025 STIX Project\n");
}

static void error_callback(const error_context_t* context) {
    fprintf(stderr, "Error: %s\n", context->message);
}

int main(int argc, char* argv[]) {
    /* Set up error handling */
    error_set_callback(error_callback);
    
    /* Command line options */
    bool show_header = false;
    bool show_sections = false;
    bool show_symbols = false;
    bool show_relocations = false;
    bool hex_dump = false;
    bool verbose = false;
    const char* filename = NULL;
    
    static struct option long_options[] = {
        {"header",      no_argument, 0, 'h'},
        {"sections",    no_argument, 0, 's'},
        {"symbols",     no_argument, 0, 'y'},
        {"relocations", no_argument, 0, 'r'},
        {"hex-dump",    no_argument, 0, 'x'},
        {"verbose",     no_argument, 0, 'v'},
        {"help",        no_argument, 0, 1001},
        {"version",     no_argument, 0, 1002},
        {0, 0, 0, 0}
    };
    
    /* Parse options */
    int opt;
    while ((opt = getopt_long(argc, argv, "hsyrxv", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                show_header = true;
                break;
            case 's':
                show_sections = true;
                break;
            case 'y':
                show_symbols = true;
                break;
            case 'r':
                show_relocations = true;
                break;
            case 'x':
                hex_dump = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 1001:
                print_usage(argv[0]);
                return EXIT_SUCCESS;
            case 1002:
                print_version();
                return EXIT_SUCCESS;
            case '?':
                fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                return EXIT_FAILURE;
        }
    }
    
    /* Get filename */
    if (optind >= argc) {
        fprintf(stderr, "Error: No input file specified\n");
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    filename = argv[optind];
    
    /* If no specific options, show everything */
    if (!show_header && !show_sections && !show_symbols && !show_relocations) {
        show_header = true;
        show_sections = true;
        show_symbols = true;
    }
    
    /* Open and read file */
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }
    
    /* Read header */
    smof_header_t header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        fprintf(stderr, "Error: Failed to read SMOF header\n");
        fclose(file);
        return EXIT_FAILURE;
    }
    
    /* Validate header */
    if (!smof_validate_header(&header)) {
        fprintf(stderr, "Error: Invalid SMOF file\n");
        fclose(file);
        return EXIT_FAILURE;
    }
    
    printf("SMOF File: %s\n", filename);
    printf("=" "============================================\n");
    
    /* Show header */
    if (show_header) {
        printf("\nFile Header:\n");
        printf("  Magic:              0x%08X ('%c%c%c%c')\n", 
               header.magic,
               (char)(header.magic & 0xFF),
               (char)((header.magic >> 8) & 0xFF),
               (char)((header.magic >> 16) & 0xFF),
               (char)((header.magic >> 24) & 0xFF));
        printf("  Version:            %u\n", header.version);
        printf("  Flags:              0x%04X", header.flags);
        
        if (header.flags & SMOF_FLAG_EXECUTABLE) printf(" EXECUTABLE");
        if (header.flags & SMOF_FLAG_RELOCATABLE) printf(" RELOCATABLE");
        if (header.flags & SMOF_FLAG_SHARED) printf(" SHARED");
        if (header.flags & SMOF_FLAG_DEBUG) printf(" DEBUG");
        if (header.flags & SMOF_FLAG_LITTLE_ENDIAN) printf(" LITTLE_ENDIAN");
        if (header.flags & SMOF_FLAG_BIG_ENDIAN) printf(" BIG_ENDIAN");
        printf("\n");
        
        printf("  Entry Point:        0x%08X\n", header.entry_point);
        printf("  Section Count:      %u\n", header.section_count);
        printf("  Symbol Count:       %u\n", header.symbol_count);
        printf("  Section Table:      0x%08X\n", header.section_table_offset);
        printf("  Symbol Table:       0x%08X\n", header.symbol_table_offset);
        printf("  String Table:       0x%08X\n", header.string_table_offset);
        printf("  Checksum:           0x%08X\n", header.checksum);
    }
    
    /* TODO: Implement section, symbol, and relocation dumping */
    
    if (show_sections) {
        printf("\nSection Headers:\n");
        printf("  [Nr] Name              Type     Flags    Addr     Off    Size   Link Info  Align\n");
        /* TODO: Read and display section headers */
    }
    
    if (show_symbols) {
        printf("\nSymbol Table:\n");
        printf("  [Nr] Value    Size Type    Bind   Vis      Ndx Name\n");
        /* TODO: Read and display symbol table */
    }
    
    if (show_relocations) {
        printf("\nRelocation Tables:\n");
        /* TODO: Read and display relocations */
    }
    
    /* Cleanup */
    fclose(file);
    
    return EXIT_SUCCESS;
}
