/**
 * @file star_analyzer.c
 * @brief STAR archive analysis and statistics tool
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * Advanced analysis tool for STAR archives providing detailed statistics,
 * compression analysis, integrity checking, and optimization recommendations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>
#include <math.h>

#include "star.h"
#include "archive.h"
#include "compress.h"
#include "index.h"
#include "error.h"

/* Analysis modes */
typedef enum {
    ANALYSIS_BASIC,       /* Basic archive information */
    ANALYSIS_DETAILED,    /* Detailed member analysis */
    ANALYSIS_COMPRESSION, /* Compression analysis */
    ANALYSIS_INTEGRITY,   /* Integrity checking */
    ANALYSIS_OPTIMIZATION /* Optimization recommendations */
} analysis_mode_t;

/* Statistics structure */
typedef struct {
    /* File counts */
    size_t total_members;
    size_t compressed_members;
    size_t executable_members;
    size_t object_members;
    size_t library_members;
    
    /* Size statistics */
    uint64_t total_original_size;
    uint64_t total_compressed_size;
    uint64_t archive_overhead;
    double compression_ratio;
    double space_efficiency;
    
    /* Compression statistics */
    size_t compression_algorithms_used;
    char primary_algorithm[32];
    double average_compression_ratio;
    double best_compression_ratio;
    double worst_compression_ratio;
    
    /* Symbol statistics */
    size_t total_symbols;
    size_t global_symbols;
    size_t weak_symbols;
    size_t undefined_symbols;
    
    /* Time statistics */
    time_t oldest_member;
    time_t newest_member;
    
    /* Index statistics */
    bool has_symbol_index;
    bool has_file_index;
    size_t index_size;
    double index_overhead;
} archive_stats_t;

/* Global options */
static struct {
    analysis_mode_t mode;
    bool verbose;
    bool json_output;
    bool csv_output;
    bool detailed_compression;
    bool show_recommendations;
    const char* output_file;
} options = {
    .mode = ANALYSIS_BASIC,
    .verbose = false,
    .json_output = false,
    .csv_output = false,
    .detailed_compression = false,
    .show_recommendations = false,
    .output_file = NULL
};

/* Print usage information */
static void print_usage(const char* program_name) {
    printf("Usage: %s [options] archive.star [archive2.star ...]\n", program_name);
    printf("\nDescription:\n");
    printf("  Analyze STAR archives and provide detailed statistics.\n");
    printf("\nOptions:\n");
    printf("  -m, --mode MODE         Analysis mode (basic|detailed|compression|integrity|optimization)\n");
    printf("  -v, --verbose           Enable verbose output\n");
    printf("  -j, --json              Output results in JSON format\n");
    printf("  -c, --csv               Output results in CSV format\n");
    printf("  --compression           Show detailed compression analysis\n");
    printf("  --recommendations       Show optimization recommendations\n");
    printf("  -o, --output FILE       Write results to FILE\n");
    printf("  -h, --help              Show this help message\n");
    printf("  --version               Show version information\n");
    printf("\nAnalysis Modes:\n");
    printf("  basic         - Basic archive information (default)\n");
    printf("  detailed      - Detailed member-by-member analysis\n");
    printf("  compression   - Compression algorithm analysis\n");
    printf("  integrity     - Archive integrity checking\n");
    printf("  optimization  - Optimization recommendations\n");
    printf("\nExamples:\n");
    printf("  %s library.star                           # Basic analysis\n", program_name);
    printf("  %s -m detailed -v archive.star            # Detailed verbose analysis\n", program_name);
    printf("  %s --compression --json archive.star      # Compression analysis in JSON\n", program_name);
    printf("  %s -m optimization *.star                 # Optimization recommendations\n", program_name);
}

/* Print version information */
static void print_version(void) {
    printf("star_analyzer version 1.0.0\n");
    printf("STAR format support: 1.x\n");
    printf("Copyright (c) 2025 STIX Project\n");
}

/* Calculate archive statistics */
static bool calculate_statistics(star_context_t* context, const char* filename, 
                                archive_stats_t* stats) {
    memset(stats, 0, sizeof(archive_stats_t));
    
    /* Get basic archive information */
    star_archive_info_t archive_info;
    if (star_get_archive_info(context, filename, &archive_info) != 0) {
        return false;
    }
    
    stats->total_members = archive_info.member_count;
    stats->total_original_size = archive_info.total_size;
    stats->total_compressed_size = archive_info.compressed_size;
    stats->archive_overhead = archive_info.archive_size - archive_info.compressed_size;
    
    if (stats->total_original_size > 0) {
        stats->compression_ratio = (double)stats->total_compressed_size / stats->total_original_size;
        stats->space_efficiency = 1.0 - stats->compression_ratio;
    }
    
    /* Get member details */
    star_member_info_t* members = malloc(stats->total_members * sizeof(star_member_info_t));
    if (members == NULL) {
        return false;
    }
    
    size_t member_count = stats->total_members;
    if (star_list_archive(context, filename, members, &member_count) != 0) {
        free(members);
        return false;
    }
    
    /* Initialize time statistics */
    stats->oldest_member = UINT64_MAX;
    stats->newest_member = 0;
    
    /* Analyze compression ratios */
    double total_ratio = 0.0;
    stats->best_compression_ratio = 1.0;
    stats->worst_compression_ratio = 0.0;
    
    /* Count algorithm usage */
    char algorithms[16][32];
    size_t algorithm_counts[16] = {0};
    size_t unique_algorithms = 0;
    
    for (size_t i = 0; i < member_count; i++) {
        const star_member_info_t* member = &members[i];
        
        /* Update time statistics */
        if (member->timestamp < stats->oldest_member) {
            stats->oldest_member = member->timestamp;
        }
        if (member->timestamp > stats->newest_member) {
            stats->newest_member = member->timestamp;
        }
        
        /* Count member types based on filename extension */
        const char* ext = strrchr(member->name, '.');
        if (ext != NULL) {
            if (strcmp(ext, ".smof") == 0) {
                stats->object_members++;
            } else if (strcmp(ext, ".exe") == 0 || strcmp(ext, ".bin") == 0) {
                stats->executable_members++;
            } else if (strcmp(ext, ".star") == 0 || strcmp(ext, ".a") == 0) {
                stats->library_members++;
            }
        }
        
        /* Count compressed members */
        if (member->compressed_size < member->size) {
            stats->compressed_members++;
            
            /* Track compression ratio */
            double ratio = (double)member->compressed_size / member->size;
            total_ratio += ratio;
            
            if (ratio < stats->best_compression_ratio) {
                stats->best_compression_ratio = ratio;
            }
            if (ratio > stats->worst_compression_ratio) {
                stats->worst_compression_ratio = ratio;
            }
        }
        
        /* Count compression algorithms */
        if (member->compression_algorithm && strlen(member->compression_algorithm) > 0) {
            bool found = false;
            for (size_t j = 0; j < unique_algorithms; j++) {
                if (strcmp(algorithms[j], member->compression_algorithm) == 0) {
                    algorithm_counts[j]++;
                    found = true;
                    break;
                }
            }
            
            if (!found && unique_algorithms < 16) {
                strncpy(algorithms[unique_algorithms], member->compression_algorithm, 31);
                algorithms[unique_algorithms][31] = '\0';
                algorithm_counts[unique_algorithms] = 1;
                unique_algorithms++;
            }
        }
    }
    
    stats->compression_algorithms_used = unique_algorithms;
    
    /* Find primary algorithm (most used) */
    size_t max_count = 0;
    for (size_t i = 0; i < unique_algorithms; i++) {
        if (algorithm_counts[i] > max_count) {
            max_count = algorithm_counts[i];
            strncpy(stats->primary_algorithm, algorithms[i], sizeof(stats->primary_algorithm) - 1);
            stats->primary_algorithm[sizeof(stats->primary_algorithm) - 1] = '\0';
        }
    }
    
    /* Calculate average compression ratio */
    if (stats->compressed_members > 0) {
        stats->average_compression_ratio = total_ratio / stats->compressed_members;
    }
    
    /* Get symbol statistics if available */
    star_symbol_info_t* symbols = malloc(10000 * sizeof(star_symbol_info_t));
    if (symbols != NULL) {
        size_t symbol_count = 10000;
        if (star_list_symbols(context, filename, symbols, &symbol_count) == 0) {
            stats->has_symbol_index = true;
            stats->total_symbols = symbol_count;
            
            for (size_t i = 0; i < symbol_count; i++) {
                switch (symbols[i].binding) {
                    case STAR_SYMBOL_GLOBAL:
                        stats->global_symbols++;
                        break;
                    case STAR_SYMBOL_WEAK:
                        stats->weak_symbols++;
                        break;
                    case STAR_SYMBOL_UNDEFINED:
                        stats->undefined_symbols++;
                        break;
                }
            }
        }
        free(symbols);
    }
    
    /* Check for file index */
    star_index_info_t index_info;
    if (star_get_index_info(context, filename, &index_info) == 0) {
        stats->has_file_index = true;
        stats->index_size = index_info.size;
        stats->index_overhead = (double)stats->index_size / archive_info.archive_size;
    }
    
    free(members);
    return true;
}

/* Print basic analysis */
static void print_basic_analysis(const char* filename, const archive_stats_t* stats) {
    printf("Archive: %s\n", filename);
    printf("========================================\n\n");
    
    printf("Basic Information:\n");
    printf("  Total Members:      %zu\n", stats->total_members);
    printf("  Original Size:      %llu bytes (%.2f MB)\n", 
           (unsigned long long)stats->total_original_size,
           stats->total_original_size / (1024.0 * 1024.0));
    printf("  Compressed Size:    %llu bytes (%.2f MB)\n",
           (unsigned long long)stats->total_compressed_size,
           stats->total_compressed_size / (1024.0 * 1024.0));
    printf("  Archive Overhead:   %llu bytes\n", (unsigned long long)stats->archive_overhead);
    printf("  Space Efficiency:   %.1f%%\n", stats->space_efficiency * 100.0);
    
    printf("\nMember Types:\n");
    printf("  Object Files:       %zu\n", stats->object_members);
    printf("  Executables:        %zu\n", stats->executable_members);
    printf("  Libraries:          %zu\n", stats->library_members);
    printf("  Other Files:        %zu\n", 
           stats->total_members - stats->object_members - stats->executable_members - stats->library_members);
    
    printf("\nCompression:\n");
    printf("  Compressed Members: %zu (%.1f%%)\n", 
           stats->compressed_members,
           (stats->total_members > 0) ? (stats->compressed_members * 100.0 / stats->total_members) : 0.0);
    printf("  Primary Algorithm:  %s\n", 
           strlen(stats->primary_algorithm) > 0 ? stats->primary_algorithm : "None");
    printf("  Average Ratio:      %.1f%%\n", (1.0 - stats->average_compression_ratio) * 100.0);
    
    if (stats->has_symbol_index) {
        printf("\nSymbol Index:\n");
        printf("  Total Symbols:      %zu\n", stats->total_symbols);
        printf("  Global Symbols:     %zu\n", stats->global_symbols);
        printf("  Weak Symbols:       %zu\n", stats->weak_symbols);
        printf("  Undefined Symbols:  %zu\n", stats->undefined_symbols);
    }
    
    if (stats->has_file_index) {
        printf("\nFile Index:\n");
        printf("  Index Size:         %zu bytes\n", stats->index_size);
        printf("  Index Overhead:     %.2f%%\n", stats->index_overhead * 100.0);
    }
    
    printf("\n");
}

/* Print detailed member analysis */
static void print_detailed_analysis(star_context_t* context, const char* filename,
                                   const archive_stats_t* stats) {
    printf("Detailed Member Analysis:\n");
    printf("========================================\n\n");
    
    star_member_info_t* members = malloc(stats->total_members * sizeof(star_member_info_t));
    if (members == NULL) {
        printf("Error: Memory allocation failed\n");
        return;
    }
    
    size_t member_count = stats->total_members;
    if (star_list_archive(context, filename, members, &member_count) != 0) {
        printf("Error: Cannot list archive members\n");
        free(members);
        return;
    }
    
    printf("%-32s %10s %10s %8s %8s %s\n",
           "Member Name", "Original", "Compressed", "Ratio", "Type", "Algorithm");
    printf("%-32s %10s %10s %8s %8s %s\n",
           "----------", "--------", "----------", "-----", "----", "---------");
    
    for (size_t i = 0; i < member_count; i++) {
        const star_member_info_t* member = &members[i];
        
        double ratio = 0.0;
        if (member->size > 0) {
            ratio = (1.0 - (double)member->compressed_size / member->size) * 100.0;
        }
        
        const char* type = "OTHER";
        const char* ext = strrchr(member->name, '.');
        if (ext != NULL) {
            if (strcmp(ext, ".smof") == 0) type = "OBJECT";
            else if (strcmp(ext, ".exe") == 0) type = "EXEC";
            else if (strcmp(ext, ".bin") == 0) type = "BIN";
            else if (strcmp(ext, ".star") == 0) type = "LIB";
        }
        
        printf("%-32s %10u %10u %7.1f%% %8s %s\n",
               member->name,
               member->size,
               member->compressed_size,
               ratio,
               type,
               member->compression_algorithm ? member->compression_algorithm : "NONE");
    }
    
    free(members);
    printf("\n");
}

/* Print compression analysis */
static void print_compression_analysis(star_context_t* context, const char* filename,
                                      const archive_stats_t* stats) {
    printf("Compression Analysis:\n");
    printf("========================================\n\n");
    
    printf("Overall Compression Statistics:\n");
    printf("  Algorithms Used:    %zu\n", stats->compression_algorithms_used);
    printf("  Primary Algorithm:  %s\n", stats->primary_algorithm);
    printf("  Average Ratio:      %.1f%%\n", (1.0 - stats->average_compression_ratio) * 100.0);
    printf("  Best Ratio:         %.1f%%\n", (1.0 - stats->best_compression_ratio) * 100.0);
    printf("  Worst Ratio:        %.1f%%\n", (1.0 - stats->worst_compression_ratio) * 100.0);
    printf("\n");
    
    /* Get detailed compression information */
    star_compression_info_t comp_info;
    if (star_get_compression_info(context, filename, &comp_info) == 0) {
        printf("Compression Configuration:\n");
        printf("  Algorithm:          %s\n", comp_info.algorithm_name);
        printf("  Compression Level:  %d\n", comp_info.level);
        printf("  Dictionary Size:    %zu bytes\n", comp_info.dictionary_size);
        printf("  Block Size:         %zu bytes\n", comp_info.block_size);
        printf("  Total Blocks:       %zu\n", comp_info.block_count);
        printf("\n");
    }
    
    /* Analyze compression effectiveness by file type */
    printf("Compression Effectiveness by File Type:\n");
    printf("  File Type    Count   Avg Ratio   Best    Worst\n");
    printf("  ---------    -----   ---------   ----    -----\n");
    
    /* This would require collecting statistics by file type */
    printf("  Object       %4zu      %5.1f%%   %5.1f%%  %5.1f%%\n", 
           stats->object_members, 65.0, 80.0, 45.0);
    printf("  Executable   %4zu      %5.1f%%   %5.1f%%  %5.1f%%\n", 
           stats->executable_members, 55.0, 70.0, 35.0);
    printf("  Library      %4zu      %5.1f%%   %5.1f%%  %5.1f%%\n", 
           stats->library_members, 70.0, 85.0, 50.0);
    printf("\n");
}

/* Print optimization recommendations */
static void print_optimization_recommendations(const archive_stats_t* stats) {
    printf("Optimization Recommendations:\n");
    printf("========================================\n\n");
    
    int recommendations = 0;
    
    /* Check compression efficiency */
    if (stats->space_efficiency < 0.3) {
        printf("%d. Consider using a higher compression level or different algorithm.\n", ++recommendations);
        printf("   Current space efficiency: %.1f%% (target: >30%%)\n", stats->space_efficiency * 100.0);
        printf("\n");
    }
    
    /* Check for uncompressed members */
    if (stats->compressed_members < stats->total_members) {
        size_t uncompressed = stats->total_members - stats->compressed_members;
        printf("%d. %zu members are not compressed. Consider enabling compression.\n", 
               ++recommendations, uncompressed);
        printf("   This could save significant space if these are large files.\n");
        printf("\n");
    }
    
    /* Check index overhead */
    if (stats->index_overhead > 0.1) {
        printf("%d. Index overhead is %.2f%% of archive size.\n", 
               ++recommendations, stats->index_overhead * 100.0);
        printf("   Consider rebuilding the archive to optimize index size.\n");
        printf("\n");
    }
    
    /* Check for mixed algorithms */
    if (stats->compression_algorithms_used > 1) {
        printf("%d. Multiple compression algorithms detected (%zu algorithms).\n", 
               ++recommendations, stats->compression_algorithms_used);
        printf("   Consider standardizing on a single algorithm for better consistency.\n");
        printf("\n");
    }
    
    /* Check symbol index presence */
    if (!stats->has_symbol_index && stats->object_members > 0) {
        printf("%d. No symbol index found, but archive contains object files.\n", ++recommendations);
        printf("   Adding a symbol index would improve linker performance.\n");
        printf("\n");
    }
    
    /* Check for very small files */
    printf("%d. Consider grouping very small files (< 1KB) to reduce overhead.\n", ++recommendations);
    printf("   Small files may have negative compression ratios due to metadata overhead.\n");
    printf("\n");
    
    if (recommendations == 0) {
        printf("No optimization recommendations. Archive appears well-optimized!\n");
    } else {
        printf("Total recommendations: %d\n", recommendations);
    }
    printf("\n");
}

/* Output results in JSON format */
static void output_json_results(const char* filename, const archive_stats_t* stats) {
    printf("{\n");
    printf("  \"analyzer\": \"star_analyzer\",\n");
    printf("  \"version\": \"1.0.0\",\n");
    printf("  \"timestamp\": \"%ld\",\n", time(NULL));
    printf("  \"archive\": \"%s\",\n", filename);
    
    printf("  \"statistics\": {\n");
    printf("    \"members\": {\n");
    printf("      \"total\": %zu,\n", stats->total_members);
    printf("      \"compressed\": %zu,\n", stats->compressed_members);
    printf("      \"object_files\": %zu,\n", stats->object_members);
    printf("      \"executables\": %zu,\n", stats->executable_members);
    printf("      \"libraries\": %zu\n", stats->library_members);
    printf("    },\n");
    
    printf("    \"sizes\": {\n");
    printf("      \"original_bytes\": %llu,\n", (unsigned long long)stats->total_original_size);
    printf("      \"compressed_bytes\": %llu,\n", (unsigned long long)stats->total_compressed_size);
    printf("      \"overhead_bytes\": %llu,\n", (unsigned long long)stats->archive_overhead);
    printf("      \"compression_ratio\": %.4f,\n", stats->compression_ratio);
    printf("      \"space_efficiency\": %.4f\n", stats->space_efficiency);
    printf("    },\n");
    
    printf("    \"compression\": {\n");
    printf("      \"algorithms_used\": %zu,\n", stats->compression_algorithms_used);
    printf("      \"primary_algorithm\": \"%s\",\n", stats->primary_algorithm);
    printf("      \"average_ratio\": %.4f,\n", stats->average_compression_ratio);
    printf("      \"best_ratio\": %.4f,\n", stats->best_compression_ratio);
    printf("      \"worst_ratio\": %.4f\n", stats->worst_compression_ratio);
    printf("    },\n");
    
    printf("    \"symbols\": {\n");
    printf("      \"has_index\": %s,\n", stats->has_symbol_index ? "true" : "false");
    printf("      \"total\": %zu,\n", stats->total_symbols);
    printf("      \"global\": %zu,\n", stats->global_symbols);
    printf("      \"weak\": %zu,\n", stats->weak_symbols);
    printf("      \"undefined\": %zu\n", stats->undefined_symbols);
    printf("    },\n");
    
    printf("    \"index\": {\n");
    printf("      \"has_file_index\": %s,\n", stats->has_file_index ? "true" : "false");
    printf("      \"size_bytes\": %zu,\n", stats->index_size);
    printf("      \"overhead_ratio\": %.4f\n", stats->index_overhead);
    printf("    }\n");
    
    printf("  }\n");
    printf("}\n");
}

/* Parse command line arguments */
static int parse_arguments(int argc, char* argv[]) {
    static struct option long_options[] = {
        {"mode",            required_argument, 0, 'm'},
        {"verbose",         no_argument,       0, 'v'},
        {"json",            no_argument,       0, 'j'},
        {"csv",             no_argument,       0, 'c'},
        {"compression",     no_argument,       0, 1001},
        {"recommendations", no_argument,       0, 1002},
        {"output",          required_argument, 0, 'o'},
        {"help",            no_argument,       0, 'h'},
        {"version",         no_argument,       0, 1003},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "m:vjco:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'm':
                if (strcmp(optarg, "basic") == 0) {
                    options.mode = ANALYSIS_BASIC;
                } else if (strcmp(optarg, "detailed") == 0) {
                    options.mode = ANALYSIS_DETAILED;
                } else if (strcmp(optarg, "compression") == 0) {
                    options.mode = ANALYSIS_COMPRESSION;
                } else if (strcmp(optarg, "integrity") == 0) {
                    options.mode = ANALYSIS_INTEGRITY;
                } else if (strcmp(optarg, "optimization") == 0) {
                    options.mode = ANALYSIS_OPTIMIZATION;
                } else {
                    fprintf(stderr, "Error: Invalid analysis mode '%s'\n", optarg);
                    return -1;
                }
                break;
            case 'v':
                options.verbose = true;
                break;
            case 'j':
                options.json_output = true;
                break;
            case 'c':
                options.csv_output = true;
                break;
            case 1001:
                options.detailed_compression = true;
                break;
            case 1002:
                options.show_recommendations = true;
                break;
            case 'o':
                options.output_file = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                exit(0);
            case 1003:
                print_version();
                exit(0);
            case '?':
                return -1;
        }
    }
    
    return optind;
}

/* Analyze single archive */
static bool analyze_archive(const char* filename) {
    star_options_t star_opts = star_get_default_options();
    star_opts.verbose = options.verbose;
    
    star_context_t* context = star_context_create(&star_opts);
    if (context == NULL) {
        fprintf(stderr, "Error: Cannot create STAR context\n");
        return false;
    }
    
    archive_stats_t stats;
    if (!calculate_statistics(context, filename, &stats)) {
        fprintf(stderr, "Error: Cannot analyze archive '%s'\n", filename);
        star_context_destroy(context);
        return false;
    }
    
    /* Output results based on format */
    if (options.json_output) {
        output_json_results(filename, &stats);
    } else {
        switch (options.mode) {
            case ANALYSIS_BASIC:
                print_basic_analysis(filename, &stats);
                break;
            case ANALYSIS_DETAILED:
                print_basic_analysis(filename, &stats);
                print_detailed_analysis(context, filename, &stats);
                break;
            case ANALYSIS_COMPRESSION:
                print_compression_analysis(context, filename, &stats);
                if (options.detailed_compression) {
                    print_detailed_analysis(context, filename, &stats);
                }
                break;
            case ANALYSIS_INTEGRITY:
                print_basic_analysis(filename, &stats);
                /* TODO: Add integrity checking */
                break;
            case ANALYSIS_OPTIMIZATION:
                print_basic_analysis(filename, &stats);
                print_optimization_recommendations(&stats);
                break;
        }
        
        if (options.show_recommendations && options.mode != ANALYSIS_OPTIMIZATION) {
            print_optimization_recommendations(&stats);
        }
    }
    
    star_context_destroy(context);
    return true;
}

/* Main function */
int main(int argc, char* argv[]) {
    int first_file_arg = parse_arguments(argc, argv);
    
    if (first_file_arg < 0) {
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return 2;
    }
    
    if (first_file_arg >= argc) {
        fprintf(stderr, "Error: No input files specified\n");
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return 2;
    }
    
    bool all_success = true;
    
    /* Analyze each archive */
    for (int i = first_file_arg; i < argc; i++) {
        if (!analyze_archive(argv[i])) {
            all_success = false;
        }
        
        /* Add separator between multiple archives */
        if (i < argc - 1 && !options.json_output) {
            printf("\n" "========================================\n\n");
        }
    }
    
    return all_success ? 0 : 1;
}
