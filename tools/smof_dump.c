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
    /* C99 compound initializer for options */
    struct {
        bool show_header;
        bool show_sections;
        bool show_symbols;
        bool show_relocations;
        bool hex_dump;
        bool verbose;
    } opts = {false, false, false, false, false, false};
    
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
    int opt;
    FILE* file;
    smof_header_t header;
    
    /* Set up error handling */
    error_set_callback(error_callback);
    
    /* Parse options */
    while ((opt = getopt_long(argc, argv, "hsyrxv", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                opts.show_header = true;
                break;
            case 's':
                opts.show_sections = true;
                break;
            case 'y':
                opts.show_symbols = true;
                break;
            case 'r':
                opts.show_relocations = true;
                break;
            case 'x':
                opts.hex_dump = true;
                break;
            case 'v':
                opts.verbose = true;
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
    if (!opts.show_header && !opts.show_sections && !opts.show_symbols && !opts.show_relocations) {
        opts.show_header = true;
        opts.show_sections = true;
        opts.show_symbols = true;
    }
    
    /* Open and read file */
    file = fopen(filename, "rb");
    if (file == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }
    
    /* Read header */
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
    if (opts.show_header) {
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
    
    /* Show sections */
    if (opts.show_sections && header.section_count > 0) {
        uint16_t i;
        
        printf("\nSection Headers:\n");
        printf("  [Nr] Name              Type     Flags    Addr     Off    Size   Link Info  Align\n");
        
        fseek(file, header.section_table_offset, SEEK_SET);
        
        for (i = 0; i < header.section_count; i++) {
            smof_section_header_t section;
            char section_name[64];
            long pos;
            
            if (fread(&section, sizeof(section), 1, file) != 1) {
                fprintf(stderr, "Error: Failed to read section header %u\n", i);
                continue;
            }
            
            /* Get section name from string table */
            strcpy(section_name, "<unknown>");
            if (header.string_table_offset > 0) {
                pos = ftell(file);
                fseek(file, header.string_table_offset + section.name_offset, SEEK_SET);
                if (fread(section_name, 1, sizeof(section_name) - 1, file) > 0) {
                    section_name[sizeof(section_name) - 1] = '\0';
                }
                fseek(file, pos, SEEK_SET);
            }
            
            printf("  [%2u] %-16s %08X %08X %08X %06X %06X %4u %4u %5u\n",
                   i, section_name, section.type, section.flags,
                   section.addr, section.offset, section.size,
                   section.link, section.info, section.alignment);
            
            /* Show hex dump if requested */
            if (opts.hex_dump && section.size > 0 && section.offset > 0) {
                uint8_t buffer[16];
                size_t remaining, to_read, read_bytes, j;
                uint32_t addr;
                unsigned char c;
                
                printf("       Hex dump of section '%s':\n", section_name);
                pos = ftell(file);
                fseek(file, section.offset, SEEK_SET);
                
                remaining = section.size;
                addr = section.addr;
                
                while (remaining > 0) {
                    to_read = remaining < 16 ? remaining : 16;
                    read_bytes = fread(buffer, 1, to_read, file);
                    
                    printf("       %08X: ", addr);
                    
                    /* Hex bytes */
                    for (j = 0; j < 16; j++) {
                        if (j < read_bytes) {
                            printf("%02X ", buffer[j]);
                        } else {
                            printf("   ");
                        }
                        if (j == 7) printf(" ");
                    }
                    
                    /* ASCII representation */
                    printf(" |");
                    for (j = 0; j < read_bytes; j++) {
                        c = buffer[j];
                        printf("%c", (c >= 32 && c <= 126) ? (char)c : '.');
                    }
                    printf("|\n");
                    
                    remaining -= read_bytes;
                    addr += (uint32_t)read_bytes;
                    
                    if (read_bytes == 0) break;
                }
                
                fseek(file, pos, SEEK_SET);
                printf("\n");
            }
        }
    }
    
    /* Show symbols */
    if (opts.show_symbols && header.symbol_count > 0) {
        uint16_t i;
        
        printf("\nSymbol Table:\n");
        printf("  [Nr] Value    Size Type    Bind   Vis      Ndx Name\n");
        
        fseek(file, header.symbol_table_offset, SEEK_SET);
        
        for (i = 0; i < header.symbol_count; i++) {
            smof_symbol_t symbol;
            char symbol_name[128];
            long pos;
            const char* type_str;
            const char* bind_str;
            const char* vis_str;
            
            if (fread(&symbol, sizeof(symbol), 1, file) != 1) {
                fprintf(stderr, "Error: Failed to read symbol %u\n", i);
                continue;
            }
            
            /* Get symbol name from string table */
            strcpy(symbol_name, "<unknown>");
            if (header.string_table_offset > 0) {
                pos = ftell(file);
                fseek(file, header.string_table_offset + symbol.name_offset, SEEK_SET);
                if (fread(symbol_name, 1, sizeof(symbol_name) - 1, file) > 0) {
                    symbol_name[sizeof(symbol_name) - 1] = '\0';
                }
                fseek(file, pos, SEEK_SET);
            }
            
            /* Decode symbol type and binding */
            type_str = "UNKNOWN";
            switch (smof_symbol_get_type(&symbol)) {
                case SMOF_STT_NOTYPE: type_str = "NOTYPE"; break;
                case SMOF_STT_OBJECT: type_str = "OBJECT"; break;
                case SMOF_STT_FUNC: type_str = "FUNC"; break;
                case SMOF_STT_SECTION: type_str = "SECTION"; break;
                case SMOF_STT_FILE: type_str = "FILE"; break;
            }
            
            bind_str = "UNKNOWN";
            switch (smof_symbol_get_binding(&symbol)) {
                case SMOF_STB_LOCAL: bind_str = "LOCAL"; break;
                case SMOF_STB_GLOBAL: bind_str = "GLOBAL"; break;
                case SMOF_STB_WEAK: bind_str = "WEAK"; break;
            }
            
            vis_str = "DEFAULT";
            /* Symbol visibility is stored in the 'other' field, but there are no defined constants for it yet */
            if ((symbol.other & 0x03) == 0) vis_str = "DEFAULT";
            else if ((symbol.other & 0x03) == 1) vis_str = "INTERNAL";
            else if ((symbol.other & 0x03) == 2) vis_str = "HIDDEN";
            else if ((symbol.other & 0x03) == 3) vis_str = "PROTECTED";
            
            printf("  [%2u] %08X %4u %-7s %-6s %-8s %3u %s\n",
                   i, symbol.value, symbol.size, type_str, bind_str, vis_str,
                   symbol.section_index, symbol_name);
        }
    }
    
    /* Show relocations */
    if (opts.show_relocations) {
        uint16_t i;
        
        printf("\nRelocation Tables:\n");
        
        /* Find relocation sections */
        fseek(file, header.section_table_offset, SEEK_SET);
        
        for (i = 0; i < header.section_count; i++) {
            smof_section_header_t section;
            char section_name[64];
            long pos;
            uint32_t rel_count, j;
            
            if (fread(&section, sizeof(section), 1, file) != 1) {
                continue;
            }
            
            if (section.type == SMOF_SECTION_REL || section.type == SMOF_SECTION_RELA) {
                /* Get section name */
                strcpy(section_name, "<unknown>");
                if (header.string_table_offset > 0) {
                    pos = ftell(file);
                    fseek(file, header.string_table_offset + section.name_offset, SEEK_SET);
                    fread(section_name, 1, sizeof(section_name) - 1, file);
                    section_name[sizeof(section_name) - 1] = '\0';
                    fseek(file, pos, SEEK_SET);
                }
                
                printf("\nRelocation section '%s' at offset 0x%x contains %u entries:\n",
                       section_name, section.offset, 
                       section.size / (uint32_t)sizeof(smof_relocation_t));
                
                printf("  Offset   Info     Type             Sym.Value Sym.Name + Addend\n");
                
                /* Read relocations */
                pos = ftell(file);
                fseek(file, section.offset, SEEK_SET);
                
                rel_count = section.size / sizeof(smof_relocation_t);
                for (j = 0; j < rel_count; j++) {
                    smof_relocation_t rel;
                    uint32_t sym_index, rel_type;
                    const char* type_str;
                    
                    if (fread(&rel, sizeof(rel), 1, file) != 1) {
                        break;
                    }
                    
                    sym_index = smof_relocation_get_symbol(rel.info);
                    rel_type = smof_relocation_get_type(rel.info);
                    
                    type_str = "UNKNOWN";
                    switch (rel_type) {
                        case SMOF_R_NONE: type_str = "NONE"; break;
                        case SMOF_R_32: type_str = "32"; break;
                        case SMOF_R_PC32: type_str = "PC32"; break;
                        case SMOF_R_16: type_str = "16"; break;
                        case SMOF_R_8: type_str = "8"; break;
                    }
                    
                    printf("  %08X %08X %-16s %08X <symbol_%u> + %d\n",
                           rel.offset, rel.info, type_str, 0, sym_index, 0);
                }
                
                fseek(file, pos, SEEK_SET);
            }
        }
    }
    
    /* Cleanup */
    fclose(file);
    
    return EXIT_SUCCESS;
}
