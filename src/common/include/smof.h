/* src/common/include/smof.h */
#ifndef SMOF_H_INCLUDED
#define SMOF_H_INCLUDED

/* C99 standard headers */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file smof.h
 * @brief STIX Minimal Object Format (SMOF) definitions
 * @details C99 compliant implementation of the SMOF format for embedded systems
 */

/* SMOF format constants */
#define SMOF_MAGIC 0x534D4F46U  /* 'SMOF' */
#define SMOF_VERSION 1U
#define SMOF_MAX_SECTIONS 255
#define SMOF_MAX_SYMBOLS 65535
#define SMOF_NAME_MAX 256

/* File flags */
#define SMOF_FLAG_EXECUTABLE    0x0001
#define SMOF_FLAG_RELOCATABLE   0x0002
#define SMOF_FLAG_SHARED        0x0004
#define SMOF_FLAG_DEBUG         0x0008
#define SMOF_FLAG_LITTLE_ENDIAN 0x0010
#define SMOF_FLAG_BIG_ENDIAN    0x0020

/* Section types */
typedef enum {
    SMOF_SECTION_NULL = 0,
    SMOF_SECTION_PROGBITS = 1,
    SMOF_SECTION_SYMTAB = 2,
    SMOF_SECTION_STRTAB = 3,
    SMOF_SECTION_RELA = 4,
    SMOF_SECTION_HASH = 5,
    SMOF_SECTION_DYNAMIC = 6,
    SMOF_SECTION_NOTE = 7,
    SMOF_SECTION_NOBITS = 8,
    SMOF_SECTION_REL = 9,
    SMOF_SECTION_DYNSYM = 11
} smof_section_type_t;

/* Section flags */
#define SMOF_SHF_WRITE     0x01
#define SMOF_SHF_ALLOC     0x02
#define SMOF_SHF_EXECINSTR 0x04
#define SMOF_SHF_MERGE     0x10
#define SMOF_SHF_STRINGS   0x20

/* Symbol binding */
typedef enum {
    SMOF_STB_LOCAL = 0,
    SMOF_STB_GLOBAL = 1,
    SMOF_STB_WEAK = 2
} smof_symbol_binding_t;

/* Symbol type */
typedef enum {
    SMOF_STT_NOTYPE = 0,
    SMOF_STT_OBJECT = 1,
    SMOF_STT_FUNC = 2,
    SMOF_STT_SECTION = 3,
    SMOF_STT_FILE = 4
} smof_symbol_type_t;

/* Relocation types */
typedef enum {
    SMOF_R_NONE = 0,
    SMOF_R_8 = 1,
    SMOF_R_16 = 2,
    SMOF_R_32 = 3,
    SMOF_R_PC8 = 4,
    SMOF_R_PC16 = 5,
    SMOF_R_PC32 = 6,
    SMOF_R_GOT32 = 7,
    SMOF_R_PLT32 = 8,
    SMOF_R_COPY = 9,
    SMOF_R_GLOB_DAT = 10,
    SMOF_R_JMP_SLOT = 11,
    SMOF_R_RELATIVE = 12
} smof_relocation_type_t;

/* C99 exact-width integer types for file format */
typedef struct smof_header {
    uint32_t magic;           /* File magic number */
    uint16_t version;         /* Format version */
    uint16_t flags;           /* File flags */
    uint32_t entry_point;     /* Entry point address */
    uint16_t section_count;   /* Number of sections */
    uint16_t symbol_count;    /* Number of symbols */
    uint32_t section_table_offset; /* Section table offset */
    uint32_t symbol_table_offset;  /* Symbol table offset */
    uint32_t string_table_offset;  /* String table offset */
    uint32_t checksum;        /* Header checksum */
} smof_header_t;

typedef struct smof_section_header {
    uint32_t name_offset;     /* Name string offset */
    uint32_t type;            /* Section type */
    uint32_t flags;           /* Section flags */
    uint32_t addr;            /* Virtual address */
    uint32_t offset;          /* File offset */
    uint32_t size;            /* Section size */
    uint32_t link;            /* Link to another section */
    uint32_t info;            /* Additional info */
    uint32_t alignment;       /* Section alignment */
    uint32_t entry_size;      /* Entry size for tables */
} smof_section_header_t;

typedef struct smof_symbol {
    uint32_t name_offset;     /* Name string offset */
    uint32_t value;           /* Symbol value */
    uint32_t size;            /* Symbol size */
    uint8_t info;             /* Type and binding */
    uint8_t other;            /* Visibility */
    uint16_t section_index;   /* Section index */
} smof_symbol_t;

typedef struct smof_relocation {
    uint32_t offset;          /* Relocation offset */
    uint32_t info;            /* Type and symbol */
} smof_relocation_t;

typedef struct smof_relocation_addend {
    uint32_t offset;          /* Relocation offset */
    uint32_t info;            /* Type and symbol */
    int32_t addend;           /* Relocation addend */
} smof_relocation_addend_t;

/* C99 static assertions for structure sizes */
_Static_assert(sizeof(smof_header_t) == 32, "SMOF header must be 32 bytes");
_Static_assert(sizeof(smof_section_header_t) == 40, "SMOF section header must be 40 bytes");
_Static_assert(sizeof(smof_symbol_t) == 16, "SMOF symbol must be 16 bytes");
_Static_assert(sizeof(smof_relocation_t) == 8, "SMOF relocation must be 8 bytes");
_Static_assert(sizeof(smof_relocation_addend_t) == 12, "SMOF relocation with addend must be 12 bytes");

/* C99 inline functions for format validation */
static inline bool smof_header_is_valid(const smof_header_t* header) {
    return header != NULL && 
           header->magic == SMOF_MAGIC && 
           header->version == SMOF_VERSION;
}

static inline uint8_t smof_symbol_get_binding(const smof_symbol_t* symbol) {
    return symbol->info >> 4;
}

static inline uint8_t smof_symbol_get_type(const smof_symbol_t* symbol) {
    return symbol->info & 0x0F;
}

static inline uint8_t smof_symbol_make_info(smof_symbol_binding_t binding, smof_symbol_type_t type) {
    return (uint8_t)((binding << 4) | (type & 0x0F));
}

static inline uint32_t smof_relocation_get_symbol(uint32_t info) {
    return info >> 8;
}

static inline uint8_t smof_relocation_get_type(uint32_t info) {
    return (uint8_t)(info & 0xFF);
}

static inline uint32_t smof_relocation_make_info(uint32_t symbol, smof_relocation_type_t type) {
    return (symbol << 8) | (uint32_t)type;
}

/* C99 designated initializers support */
extern const smof_header_t smof_default_header;

/* Function prototypes */
bool smof_validate_header(const smof_header_t* header);
uint32_t smof_calculate_checksum(const smof_header_t* header);
bool smof_is_little_endian(const smof_header_t* header);
bool smof_is_big_endian(const smof_header_t* header);

#ifdef __cplusplus
}
#endif

#endif /* SMOF_H_INCLUDED */
