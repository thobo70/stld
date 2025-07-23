/* tools/star_list.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "star.h"
#include "error.h"

/**
 * @file star_list.c
 * @brief STAR archive listing tool
 * @details Utility to list and analyze STAR archive contents
 */

static void print_usage(const char* program_name) {
    printf("Usage: %s [options] archive.star\n", program_name);
    printf("\nOptions:\n");
    printf("  -v, --verbose      Show detailed information\n");
    printf("  -s, --symbols      Show symbol index\n");
    printf("  -c, --compress     Show compression information\n");
    printf("  --help             Show this help message\n");
    printf("  --version          Show version information\n");
}

static void print_version(void) {
    printf("star_list version 1.0.0\n");
    printf("Copyright (c) 2025 STIX Project\n");
}

static void error_callback(const error_context_t* context) {
    fprintf(stderr, "Error: %s\n", context->message);
}

int main(int argc, char* argv[]) {
    /* Set up error handling */
    error_set_callback(error_callback);
    
    /* Command line options */
    bool verbose = false;
    bool show_symbols = false;
    bool show_compression = false;
    const char* filename = NULL;
    
    static struct option long_options[] = {
        {"verbose",  no_argument, 0, 'v'},
        {"symbols",  no_argument, 0, 's'},
        {"compress", no_argument, 0, 'c'},
        {"help",     no_argument, 0, 1001},
        {"version",  no_argument, 0, 1002},
        {0, 0, 0, 0}
    };
    
    /* Parse options */
    int opt;
    while ((opt = getopt_long(argc, argv, "vsc", long_options, NULL)) != -1) {
        switch (opt) {
            case 'v':
                verbose = true;
                break;
            case 's':
                show_symbols = true;
                break;
            case 'c':
                show_compression = true;
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
    
    /* Get filename */
    if (optind >= argc) {
        fprintf(stderr, "Error: No archive file specified\n");
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    filename = argv[optind];
    
    /* Create context */
    star_options_t options = star_get_default_options();
    options.verbose = verbose;
    
    star_context_t* context = star_context_create(&options);
    if (context == NULL) {
        fprintf(stderr, "Error: Failed to create archiver context\n");
        return EXIT_FAILURE;
    }
    
    printf("Archive: %s\n", filename);
    printf("=" "============================================\n");
    
    /* Get archive statistics */
    star_stats_t stats;
    int result = star_get_stats(context, filename, &stats);
    if (result == 0) {
        printf("Members:        %zu\n", stats.member_count);
        printf("Total Size:     %zu bytes\n", stats.total_size);
        printf("Compressed:     %zu bytes\n", stats.compressed_size);
        printf("Archive Size:   %zu bytes\n", stats.archive_size);
        printf("Compression:    %.1f%%\n", (1.0 - stats.compression_ratio) * 100.0);
        if (stats.symbol_count > 0) {
            printf("Symbols:        %zu\n", stats.symbol_count);
            printf("Index Size:     %zu bytes\n", stats.index_size);
        }
        printf("\n");
    }
    
    /* List members */
    star_member_info_t* members = malloc(1000 * sizeof(star_member_info_t));
    size_t count = 1000;
    
    if (members != NULL) {
        result = star_list_archive(context, filename, members, &count);
        if (result == 0) {
            if (verbose) {
                printf("%-32s %10s %10s %10s %s\n", 
                       "Name", "Size", "Compressed", "Ratio", "Date");
                printf("%-32s %10s %10s %10s %s\n", 
                       "----", "----", "----------", "-----", "----");
            } else {
                printf("Members:\n");
            }
            
            for (size_t i = 0; i < count; i++) {
                if (verbose) {
                    double ratio = 0.0;
                    if (members[i].size > 0) {
                        ratio = (1.0 - (double)members[i].compressed_size / (double)members[i].size) * 100.0;
                    }
                    
                    char time_str[64];
                    struct tm* tm_info = localtime(&members[i].timestamp);
                    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", tm_info);
                    
                    printf("%-32s %10u %10u %9.1f%% %s\n",
                           members[i].name,
                           members[i].size,
                           members[i].compressed_size,
                           ratio,
                           time_str);
                } else {
                    printf("  %s\n", members[i].name);
                }
            }
        }
        
        free(members);
    }
    
    /* TODO: Implement symbol index listing */
    if (show_symbols) {
        printf("\nSymbol Index:\n");
        /* Implementation will be added when symbol index is implemented */
    }
    
    /* Cleanup */
    star_context_destroy(context);
    
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
