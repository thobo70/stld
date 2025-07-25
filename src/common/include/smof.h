#ifndef SMOF_H
#define SMOF_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// SMOF Magic number: 'SMOF' in little-endian
#define SMOF_MAGIC 0x534D4F46U

// SMOF Version
#define SMOF_VERSION_CURRENT 1

// SMOF Header flags
#define SMOF_FLAG_EXECUTABLE    0x0001  // Executable file
#define SMOF_FLAG_SHARED_LIB    0x0002  // Shared library
#define SMOF_FLAG_POSITION_INDEP 0x0004  // Position independent
#define SMOF_FLAG_STRIPPED      0x0008  // Debug info stripped
#define SMOF_FLAG_STATIC        0x0010  // Statically linked
#define SMOF_FLAG_COMPRESSED    0x0020  // Sections are compressed
#define SMOF_FLAG_ENCRYPTED     0x0040  // Basic encryption
#define SMOF_FLAG_UNIX_FEATURES 0x0080  // Extended Unix features
#define SMOF_FLAG_LITTLE_ENDIAN 0x0100  // Little-endian byte order
#define SMOF_FLAG_BIG_ENDIAN    0x0200  // Big-endian byte order

// Section flags
#define SMOF_SECT_EXECUTABLE    0x0001  // Contains executable code
#define SMOF_SECT_WRITABLE      0x0002  // Writable at runtime
#define SMOF_SECT_READABLE      0x0004  // Readable (always set)
#define SMOF_SECT_LOADABLE      0x0008  // Should be loaded into memory
#define SMOF_SECT_ZERO_FILL     0x0010  // Fill with zeros (.bss)
#define SMOF_SECT_COMPRESSED    0x0020  // Section is compressed
#define SMOF_SECT_SHARED        0x0040  // Shareable between processes
#define SMOF_SECT_POSITION_INDEP 0x0080 // Position independent

// Symbol types
#define SMOF_SYM_NOTYPE     0    // No type specified
#define SMOF_SYM_OBJECT     1    // Data object
#define SMOF_SYM_FUNC       2    // Function
#define SMOF_SYM_SECTION    3    // Section symbol
#define SMOF_SYM_FILE       4    // File symbol
#define SMOF_SYM_SYSCALL    5    // System call symbol

// Symbol binding
#define SMOF_BIND_LOCAL     0    // Local symbol
#define SMOF_BIND_GLOBAL    1    // Global symbol
#define SMOF_BIND_WEAK      2    // Weak symbol
#define SMOF_BIND_EXPORT    3    // Exported symbol

// Relocation types
#define SMOF_RELOC_NONE       0
#define SMOF_RELOC_ABS32      1  // 32-bit absolute address
#define SMOF_RELOC_REL32      2  // 32-bit PC-relative
#define SMOF_RELOC_ABS16      3  // 16-bit absolute
#define SMOF_RELOC_REL16      4  // 16-bit PC-relative
#define SMOF_RELOC_SYSCALL    5  // System call number
#define SMOF_RELOC_GOT        6  // Global Offset Table
#define SMOF_RELOC_PLT        7  // Procedure Linkage Table

// SMOF Header structure (32 bytes)
typedef struct {
    uint32_t magic;              // 0x534D4F46 ('SMOF')
    uint16_t version;            // Format version (current: 1)
    uint16_t flags;              // File flags
    uint32_t entry_point;        // Virtual address of entry point
    uint16_t section_count;      // Number of sections
    uint16_t symbol_count;       // Number of symbols
    uint32_t string_table_offset; // Offset to string table
    uint32_t string_table_size;   // Size of string table
    uint32_t section_table_offset; // Offset to section table
    uint32_t reloc_table_offset;   // Offset to relocation table
    uint16_t reloc_count;        // Number of relocations
    uint16_t import_count;       // Number of imports
} __attribute__((packed)) smof_header_t;

// Section Table Entry (12 bytes)
typedef struct {
    uint32_t name_offset;        // Offset into string table
    uint32_t virtual_addr;       // Virtual address when loaded
    uint32_t size;               // Size in bytes
    uint32_t file_offset;        // Offset in file (0 for .bss)
    uint16_t flags;              // Section flags
    uint8_t  alignment;          // Power of 2 alignment
    uint8_t  reserved;           // Reserved for future use
} __attribute__((packed)) smof_section_t;

// Symbol Table Entry (16 bytes)
typedef struct {
    uint32_t name_offset;        // Offset into string table
    uint32_t value;              // Symbol value/address
    uint32_t size;               // Symbol size
    uint16_t section_index;      // Section index (0xFFFF = undefined)
    uint8_t  type;               // Symbol type
    uint8_t  binding;            // Symbol binding
} __attribute__((packed)) smof_symbol_t;

// Relocation Entry (8 bytes)
typedef struct {
    uint32_t offset;             // Offset within section
    uint16_t symbol_index;       // Index into symbol table
    uint8_t  type;               // Relocation type
    uint8_t  section_index;      // Section to relocate
} __attribute__((packed)) smof_relocation_t;

// Import Table Entry (8 bytes)
typedef struct {
    uint32_t name_offset;        // Library name offset
    uint32_t symbol_offset;      // Symbol name offset
} __attribute__((packed)) smof_import_t;

// Section data storage
typedef struct {
    uint8_t *data;           // Section data
    size_t size;             // Size of data
    size_t capacity;         // Allocated capacity
} smof_section_data_t;

// SMOF context for building files
typedef struct {
    smof_header_t header;
    smof_section_t *sections;
    smof_symbol_t *symbols;
    smof_relocation_t *relocations;
    smof_import_t *imports;
    smof_section_data_t *section_data;  // Data for each section
    char *string_table;
    size_t string_table_capacity;
    size_t sections_capacity;
    size_t symbols_capacity;
    size_t relocations_capacity;
    size_t imports_capacity;
} smof_context_t;

// Function declarations
// output_format_ops_t *get_smof_format(void); // STAS-specific function

// SMOF-specific functions
int smof_init_context(smof_context_t *ctx);
void smof_cleanup_context(smof_context_t *ctx);
uint32_t smof_add_string(smof_context_t *ctx, const char *str);
int smof_add_section(smof_context_t *ctx, const char *name, uint32_t virtual_addr,
                     uint32_t size, uint32_t file_offset, uint16_t flags, uint8_t alignment, 
                     const uint8_t *data);
int smof_add_symbol(smof_context_t *ctx, const char *name, uint32_t value,
                    uint32_t size, uint16_t section_index, uint8_t type, uint8_t binding);
int smof_write_file(smof_context_t *ctx, const char *filename, bool verbose);

// Validation functions
int smof_validate_header(const smof_header_t *header);
int smof_validate_section(const smof_section_t *section);

// Endianness helper functions
bool smof_is_little_endian(const smof_header_t *header);
bool smof_is_big_endian(const smof_header_t *header);

#endif // SMOF_H
