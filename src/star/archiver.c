/* src/star/archiver.c */
#include "star.h"
#include "archive.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @file archiver.c
 * @brief STAR archiver implementation
 * @details Main archiving logic implementation
 */

/* Archive context structure */
struct star_context {
    star_options_t options;
    star_progress_callback_t progress_callback;
    void* progress_user_data;
};

star_options_t star_get_default_options(void) {
    star_options_t options = {
        .compression = STAR_COMPRESS_NONE,
        .compression_level = 6,
        .create_index = true,
        .sort_members = false,
        .verbose = false,
        .force_overwrite = false,
        .max_memory = 0,
        .temp_dir = NULL
    };
    
    return options;
}

bool star_validate_options(const star_options_t* options) {
    if (options == NULL) {
        return false;
    }
    
    /* Basic validation */
    if (options->compression < STAR_COMPRESS_NONE || 
        options->compression > STAR_COMPRESS_LZMA) {
        return false;
    }
    
    if (options->compression_level < 0 || options->compression_level > 9) {
        return false;
    }
    
    return true;
}

star_context_t* star_context_create(const star_options_t* options) {
    star_context_t* context = malloc(sizeof(star_context_t));
    if (context == NULL) {
        ERROR_REPORT_ERROR(ERROR_OUT_OF_MEMORY, "Failed to allocate archiver context");
        return NULL;
    }
    
    if (options != NULL) {
        context->options = *options;
    } else {
        context->options = star_get_default_options();
    }
    
    context->progress_callback = NULL;
    context->progress_user_data = NULL;
    
    return context;
}

void star_context_destroy(star_context_t* context) {
    if (context != NULL) {
        free(context);
    }
}

void star_set_progress_callback(star_context_t* context,
                               star_progress_callback_t callback,
                               void* user_data) {
    if (context != NULL) {
        context->progress_callback = callback;
        context->progress_user_data = user_data;
    }
}

int star_create_archive(star_context_t* context,
                       const char* archive_path,
                       const char* const* file_list,
                       size_t file_count) {
    archive_file_t* archive;
    size_t i;
    int result;
    
    if (context == NULL || archive_path == NULL || file_list == NULL || file_count == 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* Report progress */
    if (context->progress_callback != NULL) {
        context->progress_callback("Creating archive", 0, context->progress_user_data);
    }
    
    /* Create archive file */
    archive = archive_create(archive_path, &context->options);
    if (archive == NULL) {
        return ERROR_FILE_IO;
    }
    
    /* Add files to archive */
    for (i = 0; i < file_count; i++) {
        result = archive_add_member_from_file(archive, file_list[i], file_list[i]);
        if (result != ERROR_SUCCESS) {
            archive_close(archive);
            return result;
        }
        
        /* Report progress */
        if (context->progress_callback != NULL) {
            int progress = (int)((i + 1) * 90 / file_count);
            context->progress_callback("Adding files", progress, context->progress_user_data);
        }
    }
    
    /* Finalize archive */
    result = archive_finalize(archive);
    archive_close(archive);
    
    if (context->progress_callback != NULL) {
        context->progress_callback("Archive creation complete", 100, context->progress_user_data);
    }
    
    return result;
}

int star_extract_archive(star_context_t* context,
                        const char* archive_path,
                        const char* output_dir,
                        const char* const* member_list,
                        size_t member_count) {
    archive_file_t* archive;
    archive_member_t* member;
    int result;
    size_t i;
    char output_path[1024];
    
    if (context == NULL || archive_path == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* Report progress */
    if (context->progress_callback != NULL) {
        context->progress_callback("Extracting archive", 0, context->progress_user_data);
    }
    
    /* Open archive for reading */
    archive = archive_open(archive_path, "rb");
    if (archive == NULL) {
        return ERROR_FILE_IO;
    }
    
    /* Load members */
    result = archive_load_members(archive);
    if (result != ERROR_SUCCESS) {
        archive_close(archive);
        return result;
    }
    
    /* Extract members */
    if (member_list == NULL || member_count == 0) {
        /* Extract all members */
        for (i = 0; i < archive->header.member_count; i++) {
            member = &archive->members[i];
            if (member->name != NULL) {
                /* Load member data if not already loaded */
                if (!member->data_loaded) {
                    member->data = malloc(member->header.size);
                    if (member->data == NULL) {
                        archive_close(archive);
                        return ERROR_OUT_OF_MEMORY;
                    }
                    
                    if (fseek(archive->file, (long)member->header.data_offset, SEEK_SET) != 0 ||
                        fread(member->data, member->header.size, 1, archive->file) != 1) {
                        archive_close(archive);
                        return ERROR_FILE_IO;
                    }
                    member->data_loaded = true;
                }
                
                /* Create output path */
                if (output_dir != NULL) {
                    snprintf(output_path, sizeof(output_path), "%s/%s", output_dir, member->name);
                } else {
                    strncpy(output_path, member->name, sizeof(output_path) - 1);
                    output_path[sizeof(output_path) - 1] = '\0';
                }
                
                /* Extract member */
                result = archive_extract_member(archive, member, output_path);
                if (result != ERROR_SUCCESS) {
                    archive_close(archive);
                    return result;
                }
                
                /* Report progress */
                if (context->progress_callback != NULL) {
                    int progress = (int)((i + 1) * 100 / archive->header.member_count);
                    context->progress_callback("Extracting files", progress, context->progress_user_data);
                }
            }
        }
    } else {
        /* Extract specific members */
        for (i = 0; i < member_count; i++) {
            member = archive_find_member(archive, member_list[i]);
            if (member == NULL) {
                archive_close(archive);
                return ERROR_MEMBER_NOT_FOUND;
            }
            
            /* Load member data if not already loaded */
            if (!member->data_loaded) {
                member->data = malloc(member->header.size);
                if (member->data == NULL) {
                    archive_close(archive);
                    return ERROR_OUT_OF_MEMORY;
                }
                
                if (fseek(archive->file, (long)member->header.data_offset, SEEK_SET) != 0 ||
                    fread(member->data, member->header.size, 1, archive->file) != 1) {
                    archive_close(archive);
                    return ERROR_FILE_IO;
                }
                member->data_loaded = true;
            }
            
            /* Create output path */
            if (output_dir != NULL) {
                snprintf(output_path, sizeof(output_path), "%s/%s", output_dir, member->name);
            } else {
                strncpy(output_path, member->name, sizeof(output_path) - 1);
                output_path[sizeof(output_path) - 1] = '\0';
            }
            
            /* Extract member */
            result = archive_extract_member(archive, member, output_path);
            if (result != ERROR_SUCCESS) {
                archive_close(archive);
                return result;
            }
            
            /* Report progress */
            if (context->progress_callback != NULL) {
                int progress = (int)((i + 1) * 100 / member_count);
                context->progress_callback("Extracting files", progress, context->progress_user_data);
            }
        }
    }
    
    archive_close(archive);
    
    if (context->progress_callback != NULL) {
        context->progress_callback("Extraction complete", 100, context->progress_user_data);
    }
    
    return ERROR_SUCCESS;
}

int star_update_archive(star_context_t* context,
                       const char* archive_path,
                       const char* const* file_list,
                       size_t file_count) {
    if (context == NULL || archive_path == NULL || file_list == NULL || file_count == 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* TODO: Implement archive update */
    return ERROR_SUCCESS;
}

int star_list_archive(star_context_t* context,
                     const char* archive_path,
                     star_member_info_t* members,
                     size_t* count) {
    archive_file_t* archive;
    int result;
    size_t max_count;
    uint32_t i;
    
    if (context == NULL || archive_path == NULL || members == NULL || count == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    max_count = *count;
    *count = 0;
    
    /* Open archive for reading */
    archive = archive_open(archive_path, "rb");
    if (archive == NULL) {
        return ERROR_FILE_IO;
    }
    
    /* Load members */
    result = archive_load_members(archive);
    if (result != ERROR_SUCCESS) {
        archive_close(archive);
        return result;
    }
    
    /* Copy member information */
    for (i = 0; i < archive->header.member_count && i < max_count; i++) {
        archive_get_member_info(&archive->members[i], &members[i]);
    }
    
    *count = archive->header.member_count;
    archive_close(archive);
    
    return ERROR_SUCCESS;
}

int star_delete_members(star_context_t* context,
                       const char* archive_path,
                       const char* const* member_list,
                       size_t member_count) {
    if (context == NULL || archive_path == NULL || member_list == NULL || member_count == 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* TODO: Implement member deletion */
    return ERROR_SUCCESS;
}

int star_get_stats(star_context_t* context,
                  const char* archive_path,
                  star_stats_t* stats) {
    if (context == NULL || archive_path == NULL || stats == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* TODO: Implement statistics collection */
    memset(stats, 0, sizeof(star_stats_t));
    
    return ERROR_SUCCESS;
}

int star_extract_member_to_memory(star_context_t* context,
                                 const char* archive_path,
                                 const char* member_name,
                                 uint8_t** data,
                                 size_t* size) {
    if (context == NULL || archive_path == NULL || member_name == NULL || 
        data == NULL || size == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* TODO: Implement memory extraction */
    *data = NULL;
    *size = 0;
    
    return ERROR_SUCCESS;
}

int star_add_member_from_memory(star_context_t* context,
                               const char* archive_path,
                               const char* member_name,
                               const uint8_t* data,
                               size_t size) {
    if (context == NULL || archive_path == NULL || member_name == NULL || 
        data == NULL || size == 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    /* TODO: Implement memory addition */
    return ERROR_SUCCESS;
}

const char* star_get_version(void) {
    return STAR_VERSION_STRING;
}
