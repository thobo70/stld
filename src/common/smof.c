/* src/common/smof.c */
#include "smof.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/**
 * @file smof.c
 * @brief SMOF format implementation (stub)
 * @details STIX Machine Object Format implementation
 */

/* Default SMOF header - STAS Original Format */
const smof_header_t smof_default_header = {
    .magic = SMOF_MAGIC,
    .version = SMOF_VERSION,
    .flags = SMOF_FLAG_LITTLE_ENDIAN,
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

bool smof_validate_header(const smof_header_t* header) {
    bool little_endian;
    bool big_endian;
    
    if (header == NULL) {
        return false;
    }
    
    /* Check magic */
    if (header->magic != SMOF_MAGIC) {
        return false;
    }
    
    /* Check version */
    if (header->version != SMOF_VERSION) {
        return false;
    }
    
    /* Check endianness flags */
    little_endian = (header->flags & SMOF_FLAG_LITTLE_ENDIAN) != 0;
    big_endian = (header->flags & SMOF_FLAG_BIG_ENDIAN) != 0;
    
    if (little_endian && big_endian) {
        return false; /* Both flags set */
    }
    
    if (!little_endian && !big_endian) {
        return false; /* Neither flag set */
    }
    
    /* Check section count */
    if (header->section_count > SMOF_MAX_SECTIONS) {
        return false;
    }
    
    /* Validate table offsets */
    if (header->section_table_offset > 0 && 
        header->section_table_offset < sizeof(smof_header_t)) {
        return false;
    }

    if (header->string_table_offset > 0 && 
        header->string_table_offset < sizeof(smof_header_t)) {
        return false;
    }

    if (header->reloc_table_offset > 0 && 
        header->reloc_table_offset < sizeof(smof_header_t)) {
        return false;
    }

    /* Check for overlapping tables */
    if (header->section_count > 0) {
        uint32_t section_table_size = header->section_count * sizeof(smof_section_header_t);
        if (header->string_table_offset > 0 && 
            header->string_table_offset < header->section_table_offset + section_table_size) {
            return false;
        }
    }

    if (header->reloc_count > 0) {
        uint32_t reloc_table_size = header->reloc_count * sizeof(smof_relocation_t);
        if (header->string_table_offset > 0 &&
            header->string_table_offset < header->reloc_table_offset + reloc_table_size) {
            return false;
        }
    }    return true;
}

bool smof_is_little_endian(const smof_header_t* header) {
    return header != NULL && (header->flags & SMOF_FLAG_LITTLE_ENDIAN) != 0;
}

bool smof_is_big_endian(const smof_header_t* header) {
    return header != NULL && (header->flags & SMOF_FLAG_BIG_ENDIAN) != 0;
}
