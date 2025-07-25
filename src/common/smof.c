/* src/common/smof.c */
#include "smof.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/**
 * @file smof.c
 * @brief SMOF format implementation (stub)
 * @details STIX Machine Object Format implementation
 */

/* Default SMOF header - STAS Compatible Format */
const smof_header_t smof_default_header = {
    .magic = SMOF_MAGIC,
    .version = SMOF_VERSION_CURRENT,
    .flags = SMOF_FLAG_EXECUTABLE | SMOF_FLAG_LITTLE_ENDIAN,
    .entry_point = 0,
    .section_count = 0,
    .symbol_count = 0,
    .string_table_offset = sizeof(smof_header_t),
    .string_table_size = 0,
    .section_table_offset = sizeof(smof_header_t),
    .reloc_table_offset = 0,
    .reloc_count = 0,
    .import_count = 0
};

int smof_validate_header(const smof_header_t* header) {
    bool little_endian;
    bool big_endian;
    
    if (header == NULL) {
        return 0;
    }
    
    /* Check magic */
    if (header->magic != SMOF_MAGIC) {
        return 0;
    }
    
    /* Check version */
    if (header->version > SMOF_VERSION_CURRENT) {
        return 0;
    }
    
    /* Check endianness flags */
    little_endian = (header->flags & SMOF_FLAG_LITTLE_ENDIAN) != 0;
    big_endian = (header->flags & SMOF_FLAG_BIG_ENDIAN) != 0;
    
    // Also check STAS-compatible endianness flags (0x0010/0x0020)
    if (!little_endian && !big_endian) {
        little_endian = (header->flags & 0x0010) != 0;  // STAS SMOF_FLAG_LITTLE_ENDIAN
        big_endian = (header->flags & 0x0020) != 0;     // STAS SMOF_FLAG_BIG_ENDIAN
    }
    
    if (little_endian && big_endian) {
        return 0; /* Both flags set */
    }
    
    if (!little_endian && !big_endian) {
        return 0; /* Neither flag set */
    }
    
    /* Sanity checks */
    if (header->section_count > 256) {
        return 0; /* Reasonable limit */
    }
    
    if (header->symbol_count > 32767) {
        return 0; /* uint16_t reasonable limit */
    }
    
    if (header->string_table_size > 1048576) {
        return 0; /* 1MB limit */
    }
    
    /* Validate table offsets */
    if (header->section_table_offset > 0 && 
        header->section_table_offset < sizeof(smof_header_t)) {
        return 0;
    }

    if (header->string_table_offset > 0 && 
        header->string_table_offset < sizeof(smof_header_t)) {
        return 0;
    }

    if (header->reloc_table_offset > 0 && 
        header->reloc_table_offset < sizeof(smof_header_t)) {
        return 0;
    }

    /* Check for overlapping tables */
    if (header->section_count > 0) {
        uint32_t section_table_size = header->section_count * sizeof(smof_section_t);
        if (header->string_table_offset > 0 && 
            header->string_table_offset > header->section_table_offset &&
            header->string_table_offset < header->section_table_offset + section_table_size) {
            return 0;
        }
    }

    if (header->reloc_count > 0) {
        uint32_t reloc_table_size = header->reloc_count * sizeof(smof_relocation_t);
        if (header->string_table_offset > 0 &&
            header->string_table_offset > header->reloc_table_offset &&
            header->string_table_offset < header->reloc_table_offset + reloc_table_size) {
            return 0;
        }
    }    
    
    return 1;
}

bool smof_is_little_endian(const smof_header_t* header) {
    return header != NULL && (header->flags & SMOF_FLAG_LITTLE_ENDIAN) != 0;
}

bool smof_is_big_endian(const smof_header_t* header) {
    return header != NULL && (header->flags & SMOF_FLAG_BIG_ENDIAN) != 0;
}
