/* src/stld/main.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "stld.h"
#include "error.h"
#include "memory.h"

/**
 * @file main.c
 * @brief STLD command line interface
 * @details C99 compliant main function for STLD linker
 */

/* Version information */
#define STLD_FULL_VERSION_STRING "STLD (STIX Linker) version " STLD_VERSION_STRING "\n"
#define STLD_COPYRIGHT "Copyright (c) 2025 STIX Project\n"

/* Command line options */
static struct option long_options[] = {
    {"output",          required_argument, 0, 'o'},
    {"library-path",    required_argument, 0, 'L'},
    {"library",         required_argument, 0, 'l'},
    {"entry",           required_argument, 0, 'e'},
    {"base-address",    required_argument, 0, 'b'},
    {"binary-flat",     no_argument,       0, 'B'},
    {"shared",          no_argument,       0, 's'},
    {"static",          no_argument,       0, 'S'},
    {"optimize-size",   no_argument,       0, 'O'},
    {"strip",           no_argument,       0, 'x'},
    {"map",             optional_argument, 0, 'm'},
    {"verbose",         no_argument,       0, 'v'},
    {"help",            no_argument,       0, 'h'},
    {"version",         no_argument,       0, 'V'},
    {0, 0, 0, 0}
};

static void print_usage(const char* program_name) {
    printf("Usage: %s [options] input-files...\n", program_name);
    printf("\nOptions:\n");
    printf("  -o, --output FILE         Write output to FILE\n");
    printf("  -L, --library-path DIR    Add DIR to library search path\n");
    printf("  -l, --library LIB         Link with library LIB\n");
    printf("  -e, --entry SYMBOL        Set entry point symbol\n");
    printf("  -b, --base-address ADDR   Set base address for binary flat output\n");
    printf("  -B, --binary-flat         Generate binary flat output\n");
    printf("  -s, --shared              Create shared library\n");
    printf("  -S, --static              Create static library\n");
    printf("  -O, --optimize-size       Optimize for size\n");
    printf("  -x, --strip               Strip debug information\n");
    printf("  -m, --map[=FILE]          Generate memory map\n");
    printf("  -v, --verbose             Enable verbose output\n");
    printf("  -h, --help                Show this help message\n");
    printf("  -V, --version             Show version information\n");
    printf("\nExamples:\n");
    printf("  %s -o program main.smof lib.smof\n", program_name);
    printf("  %s -B -b 0x100000 -o kernel.bin kernel.smof\n", program_name);
    printf("  %s -s -o libfoo.so foo.smof bar.smof\n", program_name);
}

static void print_version(void) {
    printf(STLD_FULL_VERSION_STRING);
    printf(STLD_COPYRIGHT);
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

int main(int argc, char* argv[]) {
    /* Variable declarations */
    stld_options_t options;
    const char* output_file = NULL;
    const char* map_file = NULL;
    const char** input_files;
    size_t input_count = 0;
    int opt;
    int result;
    
    /* Set up error handling */
    error_set_callback(error_callback);
    
    /* Initialize options */
    options = stld_get_default_options();
    
    /* Input files array */
    input_files = malloc((size_t)argc * sizeof(char*));
    
    if (input_files == NULL) {
        ERROR_REPORT_FATAL(ERROR_OUT_OF_MEMORY, "Failed to allocate input files array");
        return EXIT_FAILURE;
    }
    
    /* Parse command line options */
    while ((opt = getopt_long(argc, argv, "o:L:l:e:b:BsSOx::m::vhV", long_options, NULL)) != -1) {
        switch (opt) {
            case 'o':
                output_file = optarg;
                break;
                
            case 'L':
                /* TODO: Add library path */
                break;
                
            case 'l':
                /* TODO: Add library */
                break;
                
            case 'e':
                options.entry_point = (uint32_t)strtoul(optarg, NULL, 0);
                break;
                
            case 'b':
                options.base_address = (uint32_t)strtoul(optarg, NULL, 0);
                break;
                
            case 'B':
                options.output_type = STLD_OUTPUT_BINARY_FLAT;
                break;
                
            case 's':
                options.output_type = STLD_OUTPUT_SHARED_LIBRARY;
                break;
                
            case 'S':
                options.output_type = STLD_OUTPUT_STATIC_LIBRARY;
                break;
                
            case 'O':
                options.optimize = STLD_OPTIMIZE_SIZE;
                break;
                
            case 'x':
                options.strip_debug = true;
                break;
                
            case 'm':
                options.generate_map = true;
                if (optarg) {
                    map_file = optarg;
                }
                break;
                
            case 'v':
                options.verbose = true;
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
    for (int i = optind; i < argc; i++) {
        input_files[input_count++] = argv[i];
    }
    
    /* Validate arguments */
    if (input_count == 0) {
        fprintf(stderr, "Error: No input files specified\n");
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        free(input_files);
        return EXIT_FAILURE;
    }
    
    if (output_file == NULL) {
        output_file = "a.out";
    }
    
    /* Set map file if needed */
    if (options.generate_map && map_file != NULL) {
        options.map_file = map_file;
    }
    
    /* Validate options */
    if (!stld_validate_options(&options)) {
        ERROR_REPORT_ERROR(ERROR_INVALID_ARGUMENT, "Invalid linker options");
        free(input_files);
        return EXIT_FAILURE;
    }
    
    /* Perform linking */
    if (options.verbose) {
        printf("STLD: Linking %zu input files to %s\n", input_count, output_file);
    }
    
    result = stld_link_files(input_files, input_count, output_file, &options);
    
    if (result == 0) {
        if (options.verbose) {
            printf("STLD: Linking completed successfully\n");
        }
    } else {
        fprintf(stderr, "STLD: Linking failed with error code %d\n", result);
    }
    
    /* Cleanup */
    free(input_files);
    
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
