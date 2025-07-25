/* Full-featured SMOF dump tool for STAS format */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>
#include "../src/common/include/smof.h"

// Simple validation function
int smof_validate_header(const smof_header_t *header) {
    if (!header) return 0;
    
    // Magic number check
    if (header->magic != SMOF_MAGIC) return 0;
    
    // Version check
    if (header->version > SMOF_VERSION_CURRENT) return 0;
    
    // Sanity checks
    if (header->section_count > 256) return 0; // Reasonable limit
    if (header->string_table_size > 1048576) return 0; // 1MB limit
    
    // Offset validation
    if (header->section_table_offset < sizeof(smof_header_t)) return 0;
    
    return 1; // Valid
}

typedef struct {
    bool show_header;
    bool show_sections;
    bool show_symbols;
    bool show_relocations;
    bool hex_dump;
    bool verbose;
} dump_options_t;

static void print_usage(const char* program) {
    printf("Usage: %s [options] <smof_file>\n", program);
    printf("Options:\n");
    printf("  -h, --header      Show file header\n");
    printf("  -s, --sections    Show section headers\n");
    printf("  -y, --symbols     Show symbol table\n");
    printf("  -r, --relocations Show relocations\n");
    printf("  -x, --hex-dump    Show hex dump of sections\n");
    printf("  -v, --verbose     Verbose output\n");
    printf("      --help        Show this help\n");
    printf("      --version     Show version\n");
}

static void print_version(void) {
    printf("smof_dump 1.0.0 (STAS reference format)\n");
}

static const char* get_section_flag_string(uint16_t flags) {
    static char flag_str[64];
    flag_str[0] = '\0';
    
    if (flags & SMOF_SECT_WRITABLE) strcat(flag_str, "W");
    if (flags & SMOF_SECT_LOADABLE) strcat(flag_str, "A");
    if (flags & SMOF_SECT_EXECUTABLE) strcat(flag_str, "X");
    if (flags & SMOF_SECT_SHARED) strcat(flag_str, "M");
    if (flags & SMOF_SECT_READABLE) strcat(flag_str, "R");
    
    if (flag_str[0] == '\0') strcpy(flag_str, "-");
    return flag_str;
}

static const char* get_symbol_type_string(uint8_t type) {
    switch (type) {
        case SMOF_SYM_NOTYPE: return "NOTYPE";
        case SMOF_SYM_OBJECT: return "OBJECT";
        case SMOF_SYM_FUNC: return "FUNC";
        case SMOF_SYM_SECTION: return "SECTION";
        case SMOF_SYM_FILE: return "FILE";
        case SMOF_SYM_SYSCALL: return "SYSCALL";
        default: return "UNKNOWN";
    }
}

static const char* get_symbol_binding_string(uint8_t binding) {
    switch (binding) {
        case SMOF_BIND_LOCAL: return "LOCAL";
        case SMOF_BIND_GLOBAL: return "GLOBAL";
        case SMOF_BIND_WEAK: return "WEAK";
        case SMOF_BIND_EXPORT: return "EXPORT";
        default: return "UNKNOWN";
    }
}

static const char* get_relocation_type_string(uint8_t type) {
    switch (type) {
        case SMOF_RELOC_NONE: return "R_NONE";
        case SMOF_RELOC_ABS16: return "R_ABS16";
        case SMOF_RELOC_ABS32: return "R_ABS32";
        case SMOF_RELOC_REL16: return "R_REL16";
        case SMOF_RELOC_REL32: return "R_REL32";
        case SMOF_RELOC_SYSCALL: return "R_SYSCALL";
        case SMOF_RELOC_GOT: return "R_GOT";
        case SMOF_RELOC_PLT: return "R_PLT";
        default: return "R_UNKNOWN";
    }
}

static void hex_dump_section(FILE* file, const smof_section_t* section, const char* name) {
    uint8_t buffer[16];
    size_t remaining;
    uint32_t addr;
    long saved_pos;
    
    if (section->size == 0 || section->file_offset == 0) {
        printf("       (Section has no data)\n");
        return;
    }
    
    remaining = section->size;
    addr = section->virtual_addr;
    saved_pos = ftell(file);
    
    printf("       Hex dump of section '%s':\n", name);
    fseek(file, section->file_offset, SEEK_SET);
    
    while (remaining > 0) {
        size_t to_read = remaining < 16 ? remaining : 16;
        size_t read_bytes = fread(buffer, 1, to_read, file);
        size_t j;
        
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
        
        printf(" |");
        
        /* ASCII representation */
        for (j = 0; j < read_bytes; j++) {
            unsigned char c = buffer[j];
            char printable = (c >= 32 && c <= 126) ? (char)c : '.';
            printf("%c", printable);
        }
        
        printf("|\n");
        
        remaining -= read_bytes;
        addr += (uint32_t)read_bytes;
        
        if (read_bytes != to_read) break;
    }
    
    fseek(file, saved_pos, SEEK_SET);
}

int main(int argc, char* argv[]) {
    dump_options_t opts = {0};
    const char* filename;
    FILE* file;
    smof_header_t header;
    
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
    
    if (optind >= argc) {
        fprintf(stderr, "Error: No input file specified\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    filename = argv[optind];
    
    /* If no options specified, show header and sections */
    if (!opts.show_header && !opts.show_sections && !opts.show_symbols && !opts.show_relocations) {
        opts.show_header = true;
        opts.show_sections = true;
        opts.show_symbols = true;
    }
    
    /* Open file */
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
    
    printf("SMOF File: %s (STAS reference format)\n", filename);
    printf("============================================\n");
    
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
        if (header.flags & SMOF_FLAG_SHARED_LIB) printf(" SHARED_LIB");
        if (header.flags & SMOF_FLAG_POSITION_INDEP) printf(" POSITION_INDEP");
        if (header.flags & SMOF_FLAG_STRIPPED) printf(" STRIPPED");
        if (header.flags & SMOF_FLAG_STATIC) printf(" STATIC");
        if (header.flags & SMOF_FLAG_COMPRESSED) printf(" COMPRESSED");
        printf("\n");
        
        printf("  Entry Point:        0x%08X\n", header.entry_point);
        printf("  Section Count:      %u\n", header.section_count);
        printf("  Symbol Count:       %u\n", header.symbol_count);
        printf("  Section Table:      0x%08X\n", header.section_table_offset);
        printf("  String Table:       0x%08X (size: %u)\n", header.string_table_offset, header.string_table_size);
        printf("  Relocation Table:   0x%08X (%u entries)\n", header.reloc_table_offset, header.reloc_count);
        printf("  Import Count:       %u\n", header.import_count);
    }
    
    /* Show sections */
    if (opts.show_sections && header.section_count > 0) {
        printf("\nSection Headers:\n");
        printf("  [Nr] Name              VirtAddr FileOff  Size   Flags   Algn\n");
        
        fseek(file, header.section_table_offset, SEEK_SET);
        
        for (uint16_t i = 0; i < header.section_count; i++) {
            smof_section_t section;
            char section_name[64] = "<unknown>";
            long pos;
            
            if (fread(&section, sizeof(section), 1, file) != 1) {
                fprintf(stderr, "Error: Failed to read section header %u\n", i);
                continue;
            }
            
            /* Get section name from string table */
            if (header.string_table_offset > 0 && section.name_offset > 0) {
                pos = ftell(file);
                fseek(file, header.string_table_offset + section.name_offset, SEEK_SET);
                fread(section_name, 1, sizeof(section_name) - 1, file);
                section_name[sizeof(section_name) - 1] = '\0';
                fseek(file, pos, SEEK_SET);
            }
            
            printf("  [%2u] %-16s %08X %06X %06X %-7s %4u\n",
                   i, section_name, section.virtual_addr, section.file_offset,
                   section.size, get_section_flag_string(section.flags), 1U << section.alignment);
            
            /* Show hex dump if requested */
            if (opts.hex_dump) {
                hex_dump_section(file, &section, section_name);
            }
        }
    }
    
    /* Show symbols - Symbol table comes right after section table */
    if (opts.show_symbols && header.symbol_count > 0) {
        uint32_t symbol_table_offset;
        uint16_t i;
        
        printf("\nSymbol Table: %u symbols\n", header.symbol_count);
        printf("  [Nr] Value    Size Type    Bind   Ndx Name\n");
        
        /* Calculate symbol table offset: right after section table */
        symbol_table_offset = (uint32_t)(header.section_table_offset + 
                             (header.section_count * sizeof(smof_section_t)));
        
        fseek(file, symbol_table_offset, SEEK_SET);
        
        for (i = 0; i < header.symbol_count; i++) {
            smof_symbol_t symbol;
            char symbol_name[128] = "<unknown>";
            
            if (fread(&symbol, sizeof(symbol), 1, file) != 1) {
                fprintf(stderr, "Error reading symbol %u\n", i);
                break;
            }
            
            /* Read symbol name from string table */
            if (symbol.name_offset > 0 && symbol.name_offset < header.string_table_size) {
                long pos = ftell(file);
                size_t max_len = sizeof(symbol_name) - 1;
                size_t remaining = header.string_table_size - symbol.name_offset;
                size_t read_len = max_len < remaining ? max_len : remaining;
                size_t k;
                
                fseek(file, header.string_table_offset + symbol.name_offset, SEEK_SET);
                
                if (fread(symbol_name, 1, read_len, file) > 0) {
                    symbol_name[read_len] = '\0';
                    /* Ensure null termination */
                    for (k = 0; k < read_len; k++) {
                        if (symbol_name[k] == '\0') break;
                        if (symbol_name[k] < 32 || symbol_name[k] > 126) {
                            symbol_name[k] = '\0';
                            break;
                        }
                    }
                }
                
                fseek(file, pos, SEEK_SET);
            }
            
            printf("  [%2u] %08X %4u %-7s %-6s %3u %s\n",
                   i, symbol.value, symbol.size,
                   get_symbol_type_string(symbol.type),
                   get_symbol_binding_string(symbol.binding),
                   symbol.section_index,
                   symbol_name);
        }
    }
    
    /* Show relocations */
    if (opts.show_relocations && header.reloc_count > 0) {
        printf("\nRelocation Entries: %u entries\n", header.reloc_count);
        printf("  Offset   SymIdx Type     Section\n");
        
        fseek(file, header.reloc_table_offset, SEEK_SET);
        
        for (uint16_t i = 0; i < header.reloc_count; i++) {
            smof_relocation_t reloc;
            
            if (fread(&reloc, sizeof(reloc), 1, file) != 1) {
                fprintf(stderr, "Error: Failed to read relocation %u\n", i);
                continue;
            }
            
            printf("  %08X %6u %-8s %7u\n",
                   reloc.offset, reloc.symbol_index,
                   get_relocation_type_string(reloc.type),
                   reloc.section_index);
        }
    }
    
    fclose(file);
    return EXIT_SUCCESS;
}
