# SMOF (STIX Minimal Object Format) Architecture Document

## Executive Summary

The STIX Minimal Object Format (SMOF) is a custom object file format designed specifically for Unix-like systems operating under severe memory constraints (<100KB RAM). SMOF provides essential Unix functionality while maintaining an extremely small memory footprint, typically requiring only 32-96 bytes of runtime overhead.

## Design Goals

### Primary Objectives
1. **Minimal Memory Footprint**: <100 bytes runtime overhead
2. **Unix Compatibility**: Support essential Unix system features
3. **Fast Loading**: O(1) or O(n) complexity for critical operations
4. **Position Independence**: Support for memory-constrained environments
5. **Security**: Basic validation and protection mechanisms

### Essential Unix Features to Support
- Multiple text/data/bss sections
- Shared libraries (minimal dynamic linking)
- Position-independent executables (PIE)
- Symbol resolution for system calls
- Basic debugging support
- File permissions and attributes
- Process creation (fork/exec support)

## Format Architecture Overview

### File Structure Layout

```
SMOF File Structure:
┌─────────────────────────────────────┐ Offset 0
│ SMOF Header (32 bytes)              │
├─────────────────────────────────────┤ +32
│ Section Table (12 bytes × N)        │
├─────────────────────────────────────┤ +32+(12×N)
│ Symbol Table (16 bytes × M)         │
├─────────────────────────────────────┤ Variable
│ String Table                        │
├─────────────────────────────────────┤ Variable
│ Section Data                        │
│ ├─ .text (executable code)          │
│ ├─ .rodata (read-only data)         │
│ ├─ .data (initialized data)         │
│ └─ .bss (size only, no data)        │
├─────────────────────────────────────┤ Variable
│ Relocation Entries (8 bytes × R)    │
├─────────────────────────────────────┤ Variable
│ Import Table (8 bytes × I)          │ (Optional)
└─────────────────────────────────────┘
```

## Detailed Format Specification

### 1. SMOF Header (32 bytes)

```c
struct smof_header {
    uint32_t magic;           // 0x534D4F46 ('SMOF')
    uint16_t version;         // Format version (current: 1)
    uint16_t flags;           // File flags
    uint32_t entry_point;     // Virtual address of entry point
    uint16_t section_count;   // Number of sections
    uint16_t symbol_count;    // Number of symbols
    uint32_t string_table_offset; // Offset to string table
    uint32_t string_table_size;   // Size of string table
    uint32_t section_table_offset; // Offset to section table
    uint32_t reloc_table_offset;   // Offset to relocation table
    uint16_t reloc_count;     // Number of relocations
    uint16_t import_count;    // Number of imports
}; // Total: 32 bytes
```

#### Header Flags
```c
#define SMOF_FLAG_EXECUTABLE    0x0001  // Executable file
#define SMOF_FLAG_SHARED_LIB    0x0002  // Shared library
#define SMOF_FLAG_POSITION_INDEP 0x0004  // Position independent
#define SMOF_FLAG_STRIPPED      0x0008  // Debug info stripped
#define SMOF_FLAG_STATIC        0x0010  // Statically linked
#define SMOF_FLAG_COMPRESSED    0x0020  // Sections are compressed
#define SMOF_FLAG_ENCRYPTED     0x0040  // Basic encryption
#define SMOF_FLAG_UNIX_FEATURES 0x0080  // Extended Unix features
```

### 2. Section Table Entry (12 bytes)

```c
struct smof_section {
    uint32_t name_offset;     // Offset into string table
    uint32_t virtual_addr;    // Virtual address when loaded
    uint32_t size;            // Size in bytes
    uint32_t file_offset;     // Offset in file (0 for .bss)
    uint16_t flags;           // Section flags
    uint8_t  alignment;       // Power of 2 alignment
    uint8_t  reserved;        // Reserved for future use
}; // Total: 12 bytes per section
```

#### Section Flags
```c
#define SMOF_SECT_EXECUTABLE  0x0001  // Contains executable code
#define SMOF_SECT_WRITABLE    0x0002  // Writable at runtime
#define SMOF_SECT_READABLE    0x0004  // Readable (always set)
#define SMOF_SECT_LOADABLE    0x0008  // Should be loaded into memory
#define SMOF_SECT_ZERO_FILL   0x0010  // Fill with zeros (.bss)
#define SMOF_SECT_COMPRESSED  0x0020  // Section is compressed
#define SMOF_SECT_SHARED      0x0040  // Shareable between processes
#define SMOF_SECT_POSITION_INDEP 0x0080 // Position independent
```

### 3. Symbol Table Entry (16 bytes)

```c
struct smof_symbol {
    uint32_t name_offset;     // Offset into string table
    uint32_t value;           // Symbol value/address
    uint32_t size;            // Symbol size
    uint16_t section_index;   // Section index (0xFFFF = undefined)
    uint8_t  type;            // Symbol type
    uint8_t  binding;         // Symbol binding
}; // Total: 16 bytes per symbol
```

#### Symbol Types
```c
#define SMOF_SYM_NOTYPE     0    // No type specified
#define SMOF_SYM_OBJECT     1    // Data object
#define SMOF_SYM_FUNC       2    // Function
#define SMOF_SYM_SECTION    3    // Section symbol
#define SMOF_SYM_FILE       4    // File symbol
#define SMOF_SYM_SYSCALL    5    // System call symbol
```

#### Symbol Binding
```c
#define SMOF_BIND_LOCAL     0    // Local symbol
#define SMOF_BIND_GLOBAL    1    // Global symbol
#define SMOF_BIND_WEAK      2    // Weak symbol
#define SMOF_BIND_EXPORT    3    // Exported symbol
```

### 4. Relocation Entry (8 bytes)

```c
struct smof_relocation {
    uint32_t offset;          // Offset within section
    uint16_t symbol_index;    // Index into symbol table
    uint8_t  type;            // Relocation type
    uint8_t  section_index;   // Section to relocate
}; // Total: 8 bytes per relocation
```

#### Relocation Types (Architecture-specific examples for ARM/x86)
```c
// Common relocations
#define SMOF_RELOC_NONE       0
#define SMOF_RELOC_ABS32      1  // 32-bit absolute address
#define SMOF_RELOC_REL32      2  // 32-bit PC-relative
#define SMOF_RELOC_ABS16      3  // 16-bit absolute
#define SMOF_RELOC_REL16      4  // 16-bit PC-relative

// Unix-specific relocations
#define SMOF_RELOC_SYSCALL    5  // System call number
#define SMOF_RELOC_GOT        6  // Global Offset Table
#define SMOF_RELOC_PLT        7  // Procedure Linkage Table
```

### 5. Import Table Entry (8 bytes)

```c
struct smof_import {
    uint32_t name_offset;     // Library name offset
    uint32_t symbol_offset;   // Symbol name offset
}; // Total: 8 bytes per import
```

## Runtime Memory Layout

### Loader Runtime Structures

```c
// Runtime section descriptor (16 bytes)
struct smof_runtime_section {
    void*    base_addr;       // Loaded base address
    uint32_t size;            // Section size
    uint16_t flags;           // Runtime flags
    uint16_t ref_count;       // Reference count (for shared sections)
};

// Runtime symbol cache entry (12 bytes)
struct smof_runtime_symbol {
    uint32_t hash;            // Symbol name hash
    void*    address;         // Resolved address
    uint16_t flags;           // Symbol flags
    uint16_t reserved;
};

// Main loader context (64 bytes total)
struct smof_loader_context {
    struct smof_header header;              // 32 bytes
    struct smof_runtime_section* sections;  // 8 bytes (pointer)
    struct smof_runtime_symbol* symbols;    // 8 bytes (pointer)
    void* string_table;                     // 8 bytes (pointer)
    uint32_t load_base;                     // 4 bytes
    uint16_t loaded_sections;               // 2 bytes
    uint16_t resolved_symbols;              // 2 bytes
};
```

### Memory Footprint Calculation

```
Minimum SMOF Runtime Overhead:
┌─────────────────────────────────────┐
│ Loader context: 64 bytes            │
│ Section descriptors: 16 × N sections│
│ Symbol cache: 12 × M symbols        │
│ String table cache: Variable        │
└─────────────────────────────────────┘

Typical small executable (3 sections, 10 symbols):
- Loader context: 64 bytes
- Section descriptors: 48 bytes
- Symbol cache: 120 bytes
- Total: 232 bytes
```

## Unix System Integration

### 1. System Call Interface

SMOF provides special handling for Unix system calls:

```c
// System call symbol resolution
struct smof_syscall_entry {
    const char* name;         // System call name
    uint16_t    number;       // System call number
    uint8_t     arg_count;    // Number of arguments
    uint8_t     flags;        // Special flags
};

// Predefined system call table
static const struct smof_syscall_entry smof_syscalls[] = {
    {"exit",    1,  1, 0},
    {"fork",    2,  0, 0},
    {"read",    3,  3, 0},
    {"write",   4,  3, 0},
    {"open",    5,  3, 0},
    {"close",   6,  1, 0},
    {"execve",  11, 3, 0},
    // ... more system calls
};
```

### 2. Shared Library Support

Minimal dynamic linking for Unix shared libraries:

```c
// Shared library descriptor
struct smof_shared_lib {
    char name[32];            // Library name
    void* base_address;       // Loaded address
    uint32_t size;            // Library size
    uint16_t ref_count;       // Reference count
    uint16_t flags;           // Library flags
};

// Global Offset Table entry
struct smof_got_entry {
    uint32_t symbol_hash;     // Symbol name hash
    void*    address;         // Resolved address
};
```

### 3. Process Creation Support

SMOF includes metadata for Unix process creation:

```c
// Process metadata in SMOF header extension
struct smof_process_info {
    uint32_t stack_size;      // Required stack size
    uint32_t heap_size;       // Initial heap size
    uint16_t uid;             // User ID requirement
    uint16_t gid;             // Group ID requirement
    uint32_t capabilities;    // Required capabilities
    uint32_t resource_limits; // Resource limit flags
};
```

## Loader Implementation

### Core Loader Algorithm

```c
int smof_load_executable(const char* filename, struct smof_loader_context* ctx) {
    FILE* file = fopen(filename, "rb");
    if (!file) return -1;
    
    // 1. Read and validate header
    if (fread(&ctx->header, sizeof(ctx->header), 1, file) != 1) {
        fclose(file);
        return -1;
    }
    
    if (ctx->header.magic != SMOF_MAGIC) {
        fclose(file);
        return -1;
    }
    
    // 2. Allocate runtime structures
    ctx->sections = malloc(ctx->header.section_count * sizeof(struct smof_runtime_section));
    if (!ctx->sections) {
        fclose(file);
        return -1;
    }
    
    // 3. Read section table
    fseek(file, ctx->header.section_table_offset, SEEK_SET);
    struct smof_section* sections = malloc(ctx->header.section_count * sizeof(struct smof_section));
    fread(sections, sizeof(struct smof_section), ctx->header.section_count, file);
    
    // 4. Load sections into memory
    for (int i = 0; i < ctx->header.section_count; i++) {
        if (sections[i].flags & SMOF_SECT_LOADABLE) {
            smof_load_section(file, &sections[i], &ctx->sections[i]);
        }
    }
    
    // 5. Process relocations
    smof_process_relocations(file, ctx);
    
    // 6. Resolve imports
    smof_resolve_imports(file, ctx);
    
    // 7. Setup memory protection
    smof_set_memory_protection(ctx);
    
    free(sections);
    fclose(file);
    return 0;
}
```

### Section Loading

```c
int smof_load_section(FILE* file, struct smof_section* sect, 
                      struct smof_runtime_section* runtime) {
    
    // Determine memory requirements
    size_t mem_size = sect->size;
    int prot = PROT_READ;
    
    if (sect->flags & SMOF_SECT_WRITABLE) prot |= PROT_WRITE;
    if (sect->flags & SMOF_SECT_EXECUTABLE) prot |= PROT_EXEC;
    
    // Allocate memory
    void* addr = mmap(NULL, mem_size, prot, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) return -1;
    
    runtime->base_addr = addr;
    runtime->size = mem_size;
    runtime->flags = sect->flags;
    runtime->ref_count = 1;
    
    // Load section data
    if (sect->flags & SMOF_SECT_ZERO_FILL) {
        memset(addr, 0, mem_size);
    } else {
        fseek(file, sect->file_offset, SEEK_SET);
        fread(addr, 1, sect->size, file);
    }
    
    return 0;
}
```

### Relocation Processing

```c
int smof_process_relocations(FILE* file, struct smof_loader_context* ctx) {
    if (ctx->header.reloc_count == 0) return 0;
    
    // Read relocation table
    fseek(file, ctx->header.reloc_table_offset, SEEK_SET);
    struct smof_relocation* relocs = malloc(ctx->header.reloc_count * sizeof(struct smof_relocation));
    fread(relocs, sizeof(struct smof_relocation), ctx->header.reloc_count, file);
    
    // Process each relocation
    for (int i = 0; i < ctx->header.reloc_count; i++) {
        struct smof_relocation* rel = &relocs[i];
        
        // Get section base address
        void* section_base = ctx->sections[rel->section_index].base_addr;
        void* reloc_addr = (char*)section_base + rel->offset;
        
        // Resolve symbol
        void* symbol_addr = smof_resolve_symbol(ctx, rel->symbol_index);
        
        // Apply relocation
        switch (rel->type) {
            case SMOF_RELOC_ABS32:
                *(uint32_t*)reloc_addr = (uint32_t)symbol_addr;
                break;
                
            case SMOF_RELOC_REL32:
                *(uint32_t*)reloc_addr = (uint32_t)symbol_addr - (uint32_t)reloc_addr - 4;
                break;
                
            case SMOF_RELOC_SYSCALL:
                *(uint32_t*)reloc_addr = smof_get_syscall_number(ctx, rel->symbol_index);
                break;
                
            default:
                free(relocs);
                return -1;
        }
    }
    
    free(relocs);
    return 0;
}
```

## Security Features

### 1. Format Validation

```c
int smof_validate_header(struct smof_header* hdr) {
    // Magic number check
    if (hdr->magic != SMOF_MAGIC) return 0;
    
    // Version check
    if (hdr->version > SMOF_VERSION_CURRENT) return 0;
    
    // Sanity checks
    if (hdr->section_count > SMOF_MAX_SECTIONS) return 0;
    if (hdr->symbol_count > SMOF_MAX_SYMBOLS) return 0;
    if (hdr->string_table_size > SMOF_MAX_STRING_TABLE) return 0;
    
    // Offset validation
    if (hdr->section_table_offset < sizeof(struct smof_header)) return 0;
    if (hdr->string_table_offset < sizeof(struct smof_header)) return 0;
    
    return 1;
}
```

### 2. Memory Protection

```c
void smof_set_memory_protection(struct smof_loader_context* ctx) {
    for (int i = 0; i < ctx->header.section_count; i++) {
        struct smof_runtime_section* sect = &ctx->sections[i];
        
        int prot = PROT_READ;
        if (sect->flags & SMOF_SECT_WRITABLE) prot |= PROT_WRITE;
        if (sect->flags & SMOF_SECT_EXECUTABLE) prot |= PROT_EXEC;
        
        mprotect(sect->base_addr, sect->size, prot);
    }
}
```

### 3. Stack Protection

```c
// Stack canary support
#define SMOF_STACK_CANARY 0xDEADBEEF

void smof_setup_stack_protection(void* stack_base, size_t stack_size) {
    uint32_t* canary = (uint32_t*)((char*)stack_base + stack_size - sizeof(uint32_t));
    *canary = SMOF_STACK_CANARY;
}

int smof_check_stack_protection(void* stack_base, size_t stack_size) {
    uint32_t* canary = (uint32_t*)((char*)stack_base + stack_size - sizeof(uint32_t));
    return (*canary == SMOF_STACK_CANARY);
}
```

## Performance Optimizations

### 1. Symbol Hash Table

```c
// Fast symbol resolution using hash table
#define SMOF_SYMBOL_HASH_SIZE 64

struct smof_symbol_hash_entry {
    uint32_t name_hash;
    uint16_t symbol_index;
    struct smof_symbol_hash_entry* next;
};

uint32_t smof_hash_symbol_name(const char* name) {
    uint32_t hash = 5381;
    while (*name) {
        hash = ((hash << 5) + hash) + *name++;
    }
    return hash;
}
```

### 2. Lazy Loading

```c
// Lazy section loading for memory efficiency
int smof_load_section_lazy(struct smof_loader_context* ctx, int section_index) {
    if (ctx->sections[section_index].base_addr != NULL) {
        return 0; // Already loaded
    }
    
    // Load section on demand
    return smof_load_section_immediate(ctx, section_index);
}
```

### 3. Compression Support

```c
// Optional section compression
int smof_decompress_section(void* compressed_data, size_t compressed_size,
                           void* output_buffer, size_t output_size) {
    // Simple RLE decompression for .bss-like sections
    // LZ77 decompression for code sections
    // Implementation depends on compression algorithm chosen
    return smof_decompress_lz77(compressed_data, compressed_size, 
                               output_buffer, output_size);
}
```

## Unix Feature Extensions

### 1. File System Integration

```c
// SMOF file attributes for Unix integration
struct smof_file_attributes {
    uint32_t permissions;     // Unix file permissions
    uint32_t owner_uid;       // Owner user ID
    uint32_t owner_gid;       // Owner group ID
    uint64_t timestamp;       // Creation timestamp
    uint32_t checksum;        // File integrity checksum
};
```

### 2. Signal Handler Support

```c
// Signal handler metadata
struct smof_signal_handler {
    uint32_t signal_number;   // Signal number
    uint32_t handler_address; // Handler function address
    uint32_t flags;           // Handler flags
    uint32_t mask;            // Signal mask
};
```

### 3. Environment Variables

```c
// Environment variable requirements
struct smof_env_requirement {
    uint32_t name_offset;     // Variable name offset
    uint32_t default_value_offset; // Default value offset
    uint16_t flags;           // Requirement flags
    uint16_t reserved;
};
```

## Toolchain Integration

### 1. Compiler Integration

```c
// GCC plugin interface for SMOF generation
void smof_gcc_plugin_init(void) {
    register_callback("smof", PLUGIN_FINISH_UNIT, smof_generate_output, NULL);
}

void smof_generate_output(void* gcc_data, void* user_data) {
    // Extract sections from GCC internal representation
    // Generate SMOF format output
    // Optimize for memory usage
}
```

### 2. Linker Script

```ld
/* SMOF linker script */
MEMORY {
    ROM (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 96K
}

SECTIONS {
    .text : {
        *(.text*)
    } > ROM
    
    .rodata : {
        *(.rodata*)
    } > ROM
    
    .data : {
        *(.data*)
    } > RAM AT > ROM
    
    .bss : {
        *(.bss*)
    } > RAM
}

OUTPUT_FORMAT(smof)
```

### 3. Debugger Support

```c
// Minimal debugging information
struct smof_debug_info {
    uint32_t line_number_table_offset;
    uint32_t line_number_table_size;
    uint32_t source_file_table_offset;
    uint32_t source_file_table_size;
};

struct smof_line_number_entry {
    uint32_t address;         // Code address
    uint16_t line_number;     // Source line number
    uint16_t file_index;      // Source file index
};
```

## Compatibility Layer

### 1. ELF Compatibility

```c
// Minimal ELF compatibility for existing tools
int smof_to_elf_converter(const char* smof_file, const char* elf_file) {
    // Convert SMOF to minimal ELF for debugging tools
    // Preserve essential debugging information
    // Maintain symbol table compatibility
}
```

### 2. POSIX Compliance

```c
// POSIX function mapping
struct smof_posix_mapping {
    const char* posix_name;   // POSIX function name
    const char* smof_name;    // SMOF implementation name
    uint32_t    syscall_num;  // Corresponding system call
};
```

## Implementation Roadmap

### Phase 1: Core Format (Weeks 1-2)
- Basic header and section support
- Simple loader implementation
- Static linking only

### Phase 2: Unix Integration (Weeks 3-4)
- System call interface
- Process creation support
- Memory protection

### Phase 3: Dynamic Features (Weeks 5-6)
- Minimal shared library support
- Symbol resolution optimization
- Import/export mechanism

### Phase 4: Security & Optimization (Weeks 7-8)
- Security validation
- Performance optimization
- Compression support

### Phase 5: Toolchain (Weeks 9-10)
- Compiler integration
- Debugger support
- Testing and validation

## Testing Strategy

### 1. Unit Tests
- Header validation
- Section loading
- Symbol resolution
- Relocation processing

### 2. Integration Tests
- Complete executable loading
- System call functionality
- Shared library loading
- Memory protection

### 3. Performance Tests
- Load time measurement
- Memory usage profiling
- Runtime performance impact
- Comparison with ELF/XCOFF

### 4. Compatibility Tests
- Unix system integration
- POSIX compliance
- Existing toolchain compatibility

## Conclusion

The Unix Minimal Object Format (UMOF) provides a balanced solution for Unix-like systems operating under severe memory constraints. With a typical runtime overhead of only 200-300 bytes and support for essential Unix features, UMOF enables full Unix functionality while maintaining the memory efficiency required for embedded and resource-constrained systems.

Key advantages:
- **Minimal overhead**: 32-byte header, <300 bytes runtime
- **Unix compatibility**: Full system call support, shared libraries
- **Security**: Memory protection, format validation
- **Performance**: Fast loading, efficient symbol resolution
- **Extensibility**: Designed for future Unix feature additions

UMOF represents an optimal balance between functionality and memory efficiency for modern Unix systems operating in constrained environments.
