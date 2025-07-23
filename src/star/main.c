/* src/star/main.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include "star.h"
#include "error.h"

/**
 * @file main.c
 * @brief STAR command line interface
 * @details C99 compliant main function for STAR archiver
 */

/* Version information */
#define STAR_FULL_VERSION_STRING "STAR (STIX Archiver) version " STAR_VERSION_STRING "\n"
#define STAR_COPYRIGHT "Copyright (c) 2025 STIX Project\n"

/* Command line options */
static struct option long_options[] = {
    {"create",          no_argument,       0, 'c'},
    {"extract",         no_argument,       0, 'x'},
    {"update",          no_argument,       0, 'u'},
    {"list",            no_argument,       0, 't'},
    {"delete",          no_argument,       0, 'd'},
    {"file",            required_argument, 0, 'f'},
    {"directory",       required_argument, 0, 'C'},
    {"compress",        required_argument, 0, 'z'},
    {"level",           required_argument, 0, 'L'},
    {"index",           no_argument,       0, 'i'},
    {"sort",            no_argument,       0, 's'},
    {"verbose",         no_argument,       0, 'v'},
    {"force",           no_argument,       0, 'F'},
    {"help",            no_argument,       0, 'h'},
    {"version",         no_argument,       0, 'V'},
    {0, 0, 0, 0}
};

static void print_usage(const char* program_name) {
    printf("Usage: %s [options] [files...]\n", program_name);
    printf("\nOperations:\n");
    printf("  -c, --create              Create new archive\n");
    printf("  -x, --extract             Extract from archive\n");
    printf("  -u, --update              Update archive\n");
    printf("  -t, --list                List archive contents\n");
    printf("  -d, --delete              Delete members from archive\n");
    printf("\nOptions:\n");
    printf("  -f, --file ARCHIVE        Use ARCHIVE file\n");
    printf("  -C, --directory DIR       Change to DIR before operation\n");
    printf("  -z, --compress ALG        Use compression algorithm (none|lz4|zlib|lzma)\n");
    printf("  -L, --level LEVEL         Set compression level (0-9)\n");
    printf("  -i, --index               Create symbol index\n");
    printf("  -s, --sort                Sort members by name\n");
    printf("  -v, --verbose             Enable verbose output\n");
    printf("  -F, --force               Force overwrite existing files\n");
    printf("  -h, --help                Show this help message\n");
    printf("  -V, --version             Show version information\n");
    printf("\nExamples:\n");
    printf("  %s -cf library.star *.o          # Create archive\n", program_name);
    printf("  %s -tf library.star               # List contents\n", program_name);
    printf("  %s -xf library.star               # Extract all\n", program_name);
    printf("  %s -xf library.star file.o        # Extract specific file\n", program_name);
    printf("  %s -czf library.star.lz4 *.o      # Create with LZ4 compression\n", program_name);
}

static void print_version(void) {
    printf(STAR_FULL_VERSION_STRING);
    printf(STAR_COPYRIGHT);
    printf("This is free software; see the source for copying conditions.\n");
}

static void error_callback(const error_context_t* context) {
    const char* severity = "";
    switch (context->severity) {
        case ERROR_SEVERITY_INFO:    severity = "Info"; break;
        case ERROR_SEVERITY_WARNING: severity = "Warning"; break;
        case ERROR_SEVERITY_ERROR:   severity = "Error"; break;
        case ERROR_SEVERITY_FATAL:   severity = "Fatal"; break;
    }
    
    fprintf(stderr, "%s: %s (%s:%d in %s)\n",
            severity, context->message,
            context->file, context->line, context->function);
}

static void progress_callback(const char* operation, int progress, void* user_data) {
    bool verbose = *(bool*)user_data;
    if (verbose) {
        printf("\r%s: %d%%", operation, progress);
        fflush(stdout);
        if (progress == 100) {
            printf("\n");
        }
    }
}

static star_compression_t parse_compression(const char* name) {
    if (strcmp(name, "none") == 0) {
        return STAR_COMPRESS_NONE;
    } else if (strcmp(name, "lz4") == 0) {
        return STAR_COMPRESS_LZ4;
    } else if (strcmp(name, "zlib") == 0) {
        return STAR_COMPRESS_ZLIB;
    } else if (strcmp(name, "lzma") == 0) {
        return STAR_COMPRESS_LZMA;
    } else {
        return STAR_COMPRESS_NONE;
    }
}

int main(int argc, char* argv[]) {
    /* Variable declarations */
    star_options_t options;
    star_mode_t mode;
    const char* archive_file = NULL;
    const char* directory = NULL;
    const char** input_files;
    size_t input_count = 0;
    int opt;
    star_context_t* context;
    int result = 0;
    int i;
    bool mode_set = false;
    
    /* Set up error handling */
    error_set_callback(error_callback);
    
    /* Initialize options */
    options = star_get_default_options();
    mode_set = false;
    
    /* Input files array */
    input_files = malloc((size_t)argc * sizeof(char*));
    
    if (input_files == NULL) {
        ERROR_REPORT_FATAL(ERROR_OUT_OF_MEMORY, "Failed to allocate input files array");
        return EXIT_FAILURE;
    }
    
    /* Parse command line options */
    while ((opt = getopt_long(argc, argv, "cxutdf:C:z:L:isvFhV", long_options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                mode = STAR_MODE_CREATE;
                mode_set = true;
                break;
                
            case 'x':
                mode = STAR_MODE_EXTRACT;
                mode_set = true;
                break;
                
            case 'u':
                mode = STAR_MODE_UPDATE;
                mode_set = true;
                break;
                
            case 't':
                mode = STAR_MODE_LIST;
                mode_set = true;
                break;
                
            case 'd':
                mode = STAR_MODE_DELETE;
                mode_set = true;
                break;
                
            case 'f':
                archive_file = optarg;
                break;
                
            case 'C':
                directory = optarg;
                break;
                
            case 'z':
                options.compression = parse_compression(optarg);
                break;
                
            case 'L':
                options.compression_level = atoi(optarg);
                break;
                
            case 'i':
                options.create_index = true;
                break;
                
            case 's':
                options.sort_members = true;
                break;
                
            case 'v':
                options.verbose = true;
                break;
                
            case 'F':
                options.force_overwrite = true;
                break;
                
            case 'h':
                print_usage(argv[0]);
                free(input_files);
                return EXIT_SUCCESS;
                
            case 'V':
                print_version();
                free(input_files);
                return EXIT_SUCCESS;
                
            case '?':
                fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                free(input_files);
                return EXIT_FAILURE;
                
            default:
                break;
        }
    }
    
    /* Collect input files */
    for (i = optind; i < argc; i++) {
        input_files[input_count++] = argv[i];
    }
    
    /* Validate arguments */
    if (!mode_set) {
        fprintf(stderr, "Error: No operation specified\n");
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        free(input_files);
        return EXIT_FAILURE;
    }
    
    if (archive_file == NULL) {
        fprintf(stderr, "Error: No archive file specified\n");
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        free(input_files);
        return EXIT_FAILURE;
    }
    
    /* Change directory if specified */
    if (directory != NULL) {
        if (chdir(directory) != 0) {
            perror("chdir");
            free(input_files);
            return EXIT_FAILURE;
        }
    }
    
    /* Validate options */
    if (!star_validate_options(&options)) {
        ERROR_REPORT_ERROR(ERROR_INVALID_ARGUMENT, "Invalid archiver options");
        free(input_files);
        return EXIT_FAILURE;
    }
    
    /* Create context */
    context = star_context_create(&options);
    if (context == NULL) {
        ERROR_REPORT_FATAL(ERROR_OUT_OF_MEMORY, "Failed to create archiver context");
        free(input_files);
        return EXIT_FAILURE;
    }
    
    /* Set progress callback */
    star_set_progress_callback(context, progress_callback, &options.verbose);
    
    /* Perform operation */
    result = 0;
    
    switch (mode) {
        case STAR_MODE_CREATE:
            if (input_count == 0) {
                fprintf(stderr, "Error: No files specified for archive creation\n");
                result = EXIT_FAILURE;
            } else {
                result = star_create_archive(context, archive_file, input_files, input_count);
            }
            break;
            
        case STAR_MODE_EXTRACT:
            result = star_extract_archive(context, archive_file, directory, 
                                        input_files, input_count);
            break;
            
        case STAR_MODE_UPDATE:
            if (input_count == 0) {
                fprintf(stderr, "Error: No files specified for archive update\n");
                result = EXIT_FAILURE;
            } else {
                result = star_update_archive(context, archive_file, input_files, input_count);
            }
            break;
            
        case STAR_MODE_LIST: {
            star_member_info_t* members = malloc(1000 * sizeof(star_member_info_t));
            size_t count = 1000;
            
            if (members != NULL) {
                result = star_list_archive(context, archive_file, members, &count);
                if (result == 0) {
                    printf("Archive: %s\n", archive_file);
                    for (size_t j = 0; j < count; j++) {
                        printf("  %s (%u bytes)\n", members[j].name, members[j].size);
                    }
                }
                free(members);
            }
            break;
        }
        
        case STAR_MODE_DELETE:
            if (input_count == 0) {
                fprintf(stderr, "Error: No members specified for deletion\n");
                result = EXIT_FAILURE;
            } else {
                result = star_delete_members(context, archive_file, input_files, input_count);
            }
            break;
    }
    
    if (result == 0) {
        if (options.verbose) {
            printf("STAR: Operation completed successfully\n");
        }
    } else {
        fprintf(stderr, "STAR: Operation failed with error code %d\n", result);
    }
    
    /* Cleanup */
    star_context_destroy(context);
    free(input_files);
    
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
