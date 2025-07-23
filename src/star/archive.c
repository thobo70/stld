/* src/star/archive.c */
#include "archive.h"
#include "star.h"
#include "../common/include/error.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>

/**
 * @file archive.c
 * @brief Archive file format implementation
 * @details Core archive I/O operations for STAR format
 */

/* Calculate CRC32 checksum */
static uint32_t crc32_table[256];
static bool crc32_initialized = false;

static void init_crc32_table(void) {
    uint32_t c;
    int n, k;
    
    if (crc32_initialized) return;
    
    for (n = 0; n < 256; n++) {
        c = (uint32_t)n;
        for (k = 0; k < 8; k++) {
            if (c & 1) {
                c = 0xedb88320L ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        crc32_table[n] = c;
    }
    crc32_initialized = true;
}

uint32_t archive_calculate_checksum(const void* data, size_t size) {
    const uint8_t* buf = (const uint8_t*)data;
    uint32_t crc = 0xffffffffL;
    size_t i;
    
    init_crc32_table();
    
    for (i = 0; i < size; i++) {
        crc = crc32_table[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
    }
    
    return crc ^ 0xffffffffL;
}

bool archive_validate_header(const star_header_t* header) {
    if (header == NULL) {
        return false;
    }
    
    /* Check magic */
    if (header->magic != STAR_MAGIC) {
        return false;
    }
    
    /* Check version */
    if (header->version != STAR_VERSION) {
        return false;
    }
    
    /* Check member count */
    if (header->member_count > STAR_MAX_MEMBERS) {
        return false;
    }
    
    /* Validate offsets */
    if (header->member_table_offset > 0 && 
        header->member_table_offset < sizeof(star_header_t)) {
        return false;
    }
    
    if (header->string_table_offset > 0 && 
        header->string_table_offset < sizeof(star_header_t)) {
        return false;
    }
    
    return true;
}

archive_file_t* archive_open(const char* filename, const char* mode) {
    archive_file_t* archive;
    FILE* file;
    bool writable = false;
    
    if (filename == NULL || mode == NULL) {
        return NULL;
    }
    
    /* Determine access mode */
    if (strchr(mode, 'w') || strchr(mode, 'a') || strchr(mode, '+')) {
        writable = true;
    }
    
    file = fopen(filename, mode);
    if (file == NULL) {
        return NULL;
    }
    
    archive = malloc(sizeof(archive_file_t));
    if (archive == NULL) {
        fclose(file);
        return NULL;
    }
    
    memset(archive, 0, sizeof(archive_file_t));
    archive->file = file;
    archive->is_open = true;
    archive->is_writable = writable;
    archive->filename = filename;
    
    /* If reading existing archive, load header */
    if (!writable || strchr(mode, '+')) {
        if (fread(&archive->header, sizeof(star_header_t), 1, file) == 1) {
            if (!archive_validate_header(&archive->header)) {
                archive_close(archive);
                return NULL;
            }
        } else {
            /* New file or empty file - initialize header for writing */
            if (writable) {
                memset(&archive->header, 0, sizeof(star_header_t));
                archive->header.magic = STAR_MAGIC;
                archive->header.version = STAR_VERSION;
                archive->header.creation_time = (uint32_t)time(NULL);
            } else {
                archive_close(archive);
                return NULL;
            }
        }
    }
    
    return archive;
}

void archive_close(archive_file_t* archive) {
    if (archive == NULL) {
        return;
    }
    
    if (archive->file != NULL) {
        fclose(archive->file);
    }
    
    if (archive->members != NULL) {
        for (uint32_t i = 0; i < archive->header.member_count; i++) {
            free(archive->members[i].name);
            free(archive->members[i].data);
        }
        free(archive->members);
    }
    
    free(archive->string_table);
    free(archive->symbols);
    free(archive);
}

bool archive_is_valid(const archive_file_t* archive) {
    return archive != NULL && 
           archive->is_open && 
           archive->file != NULL &&
           archive_validate_header(&archive->header);
}

archive_file_t* archive_create(const char* filename, const star_options_t* options) {
    archive_file_t* archive;
    
    if (filename == NULL) {
        return NULL;
    }
    
    archive = archive_open(filename, "wb+");
    if (archive == NULL) {
        return NULL;
    }
    
    /* Initialize header */
    archive->header.magic = STAR_MAGIC;
    archive->header.version = STAR_VERSION;
    archive->header.member_count = 0;
    archive->header.creation_time = (uint32_t)time(NULL);
    
    /* Set flags based on options */
    if (options != NULL) {
        if (options->compression != STAR_COMPRESS_NONE) {
            archive->header.flags |= STAR_FLAG_COMPRESSED;
        }
        if (options->create_index) {
            archive->header.flags |= STAR_FLAG_INDEXED;
        }
        if (options->sort_members) {
            archive->header.flags |= STAR_FLAG_SORTED;
        }
    }
    
    /* Set endianness flag */
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    archive->header.flags |= STAR_FLAG_LITTLE_ENDIAN;
    #else
    archive->header.flags |= STAR_FLAG_BIG_ENDIAN;
    #endif
    
    /* Allocate initial string table */
    archive->string_table = malloc(1024);
    if (archive->string_table == NULL) {
        archive_close(archive);
        return NULL;
    }
    archive->string_table[0] = '\0'; /* Empty string at offset 0 */
    archive->header.string_table_size = 1;
    
    return archive;
}

int archive_add_string(archive_file_t* archive, const char* str, uint32_t* offset) {
    size_t len;
    size_t new_size;
    char* new_table;
    
    if (archive == NULL || str == NULL || offset == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* Check if string already exists */
    for (uint32_t i = 0; i < archive->header.string_table_size; i++) {
        if (strcmp(&archive->string_table[i], str) == 0) {
            *offset = i;
            return ERROR_SUCCESS;
        }
        /* Skip to next string */
        while (i < archive->header.string_table_size && archive->string_table[i] != '\0') {
            i++;
        }
    }
    
    /* Add new string */
    len = strlen(str) + 1;
    new_size = archive->header.string_table_size + len;
    
    new_table = realloc(archive->string_table, new_size);
    if (new_table == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }
    
    archive->string_table = new_table;
    *offset = archive->header.string_table_size;
    
    strcpy(&archive->string_table[*offset], str);
    archive->header.string_table_size = (uint32_t)new_size;
    
    return ERROR_SUCCESS;
}

const char* archive_get_string(const archive_file_t* archive, uint32_t offset) {
    if (archive == NULL || archive->string_table == NULL || 
        offset >= archive->header.string_table_size) {
        return NULL;
    }
    
    return &archive->string_table[offset];
}

int archive_add_member_from_file(archive_file_t* archive,
                                const char* member_name,
                                const char* file_path) {
    FILE* input_file;
    struct stat st;
    archive_member_t* new_members;
    archive_member_t* member;
    uint8_t* data;
    size_t bytes_read;
    uint32_t name_offset;
    int result;
    
    if (archive == NULL || member_name == NULL || file_path == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    if (!archive->is_writable) {
        return ERROR_PERMISSION_DENIED;
    }
    
    /* Get file info */
    if (stat(file_path, &st) != 0) {
        return ERROR_FILE_IO;
    }
    
    /* Open input file */
    input_file = fopen(file_path, "rb");
    if (input_file == NULL) {
        return ERROR_FILE_IO;
    }
    
    /* Allocate memory for file data */
    data = malloc((size_t)st.st_size);
    if (data == NULL) {
        fclose(input_file);
        return ERROR_OUT_OF_MEMORY;
    }
    
    /* Read file data */
    bytes_read = fread(data, 1, (size_t)st.st_size, input_file);
    fclose(input_file);
    
    if (bytes_read != (size_t)st.st_size) {
        free(data);
        return ERROR_FILE_IO;
    }
    
    /* Add member name to string table */
    result = archive_add_string(archive, member_name, &name_offset);
    if (result != ERROR_SUCCESS) {
        free(data);
        return result;
    }
    
    /* Expand members array */
    new_members = realloc(archive->members, 
                         (archive->header.member_count + 1) * sizeof(archive_member_t));
    if (new_members == NULL) {
        free(data);
        return ERROR_OUT_OF_MEMORY;
    }
    
    archive->members = new_members;
    member = &archive->members[archive->header.member_count];
    
    /* Initialize member */
    memset(member, 0, sizeof(archive_member_t));
    member->header.name_offset = name_offset;
    member->header.size = (uint32_t)st.st_size;
    member->header.compressed_size = (uint32_t)st.st_size; /* No compression for now */
    member->header.timestamp = (uint32_t)st.st_mtime;
    member->header.checksum = archive_calculate_checksum(data, (size_t)st.st_size);
    member->name = malloc(strlen(member_name) + 1);
    member->data = data;
    member->data_loaded = true;
    member->index = archive->header.member_count;
    
    if (member->name == NULL) {
        free(data);
        return ERROR_OUT_OF_MEMORY;
    }
    
    strcpy(member->name, member_name);
    archive->header.member_count++;
    
    return ERROR_SUCCESS;
}

int archive_write_header(archive_file_t* archive) {
    if (archive == NULL || !archive->is_writable) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* Calculate offsets */
    archive->header.member_table_offset = sizeof(star_header_t);
    archive->header.string_table_offset = archive->header.member_table_offset + 
                                         (uint32_t)(archive->header.member_count * sizeof(star_member_header_t));
    
    /* Calculate checksum (excluding checksum field) */
    archive->header.checksum = archive_calculate_checksum(&archive->header, 
                                                         sizeof(star_header_t) - sizeof(uint32_t));
    
    /* Write header */
    if (fseek(archive->file, 0, SEEK_SET) != 0) {
        return ERROR_FILE_IO;
    }
    
    if (fwrite(&archive->header, sizeof(star_header_t), 1, archive->file) != 1) {
        return ERROR_FILE_IO;
    }
    
    return ERROR_SUCCESS;
}

int archive_finalize(archive_file_t* archive) {
    uint32_t data_offset;
    uint32_t i;
    
    if (archive == NULL || !archive->is_writable) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* Calculate offsets first */
    archive->header.member_table_offset = sizeof(star_header_t);
    archive->header.string_table_offset = archive->header.member_table_offset + 
                                         (uint32_t)(archive->header.member_count * sizeof(star_member_header_t));
    
    /* Calculate data offset */
    data_offset = archive->header.string_table_offset + archive->header.string_table_size;

    /* Write member headers */
    if (fseek(archive->file, (long)archive->header.member_table_offset, SEEK_SET) != 0) {
        return ERROR_FILE_IO;
    }

    for (i = 0; i < archive->header.member_count; i++) {
        archive->members[i].header.data_offset = data_offset;
        data_offset += archive->members[i].header.size;
        
        if (fwrite(&archive->members[i].header, sizeof(star_member_header_t), 1, archive->file) != 1) {
            return ERROR_FILE_IO;
        }
    }
    fflush(archive->file);    /* Write string table */
    if (fseek(archive->file, (long)archive->header.string_table_offset, SEEK_SET) != 0) {
        return ERROR_FILE_IO;
    }

    if (fwrite(archive->string_table, archive->header.string_table_size, 1, archive->file) != 1) {
        return ERROR_FILE_IO;
    }
    fflush(archive->file);

    /* Write member data */
    for (i = 0; i < archive->header.member_count; i++) {
        if (archive->members[i].data_loaded) {
            if (fwrite(archive->members[i].data, archive->members[i].header.size, 1, archive->file) != 1) {
                return ERROR_FILE_IO;
            }
        }
    }
    fflush(archive->file);    /* Write header last */
    return archive_write_header(archive);
}

archive_member_t* archive_find_member(const archive_file_t* archive, const char* name) {
    uint32_t i;
    
    if (archive == NULL || name == NULL) {
        return NULL;
    }
    
    for (i = 0; i < archive->header.member_count; i++) {
        if (archive->members[i].name != NULL && 
            strcmp(archive->members[i].name, name) == 0) {
            return &archive->members[i];
        }
    }
    
    return NULL;
}

archive_member_t* archive_get_member(const archive_file_t* archive, uint32_t index) {
    if (archive == NULL || index >= archive->header.member_count) {
        return NULL;
    }
    
    return &archive->members[index];
}

int archive_extract_member(const archive_file_t* archive,
                          const archive_member_t* member,
                          const char* output_path) {
    FILE* output_file;
    
    if (archive == NULL || member == NULL || output_path == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    if (!member->data_loaded) {
        return ERROR_INVALID_ARGUMENT; /* TODO: Load data from file */
    }
    
    output_file = fopen(output_path, "wb");
    if (output_file == NULL) {
        return ERROR_FILE_IO;
    }
    
    if (fwrite(member->data, member->header.size, 1, output_file) != 1) {
        fclose(output_file);
        return ERROR_FILE_IO;
    }
    
    fclose(output_file);
    return ERROR_SUCCESS;
}

int archive_load_members(archive_file_t* archive) {
    uint32_t i;
    star_member_header_t member_header;
    archive_member_t* member;
    
    if (archive == NULL || archive->header.member_count == 0) {
        return ERROR_SUCCESS;
    }
    
    /* Allocate members array */
    archive->members = malloc(archive->header.member_count * sizeof(archive_member_t));
    if (archive->members == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }
    
    /* Load string table */
    if (archive->header.string_table_size > 0) {
        archive->string_table = malloc(archive->header.string_table_size);
        if (archive->string_table == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }
        
        if (fseek(archive->file, (long)archive->header.string_table_offset, SEEK_SET) != 0 ||
            fread(archive->string_table, archive->header.string_table_size, 1, archive->file) != 1) {
            return ERROR_FILE_IO;
        }
    }
    
    /* Load member headers */
    if (fseek(archive->file, (long)archive->header.member_table_offset, SEEK_SET) != 0) {
        return ERROR_FILE_IO;
    }
    
    for (i = 0; i < archive->header.member_count; i++) {
        if (fread(&member_header, sizeof(star_member_header_t), 1, archive->file) != 1) {
            return ERROR_FILE_IO;
        }
        
        member = &archive->members[i];
        member->header = member_header;
        member->index = i;
        member->data = NULL;
        member->data_loaded = false;
        
        /* Get member name from string table */
        if (member_header.name_offset < archive->header.string_table_size) {
            const char* name = &archive->string_table[member_header.name_offset];
            member->name = malloc(strlen(name) + 1);
            if (member->name != NULL) {
                strcpy(member->name, name);
            }
        } else {
            member->name = NULL;
        }
    }
    
    return ERROR_SUCCESS;
}

void archive_get_member_info(const archive_member_t* member, star_member_info_t* info) {
    if (member == NULL || info == NULL) {
        return;
    }
    
    memset(info, 0, sizeof(star_member_info_t));
    
    if (member->name != NULL) {
        strncpy(info->name, member->name, STAR_MEMBER_NAME_MAX - 1);
        info->name[STAR_MEMBER_NAME_MAX - 1] = '\0';
    }
    
    info->size = member->header.size;
    info->compressed_size = member->header.compressed_size;
    info->timestamp = (time_t)member->header.timestamp;
    info->flags = member->header.flags;
    info->compression = (star_compression_t)member->header.compression;
    info->checksum = member->header.checksum;
}
