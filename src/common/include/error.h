/* src/common/include/error.h */
#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file error.h
 * @brief Error handling system for STLD/STAR
 * @details C99 compliant error handling with embedded systems support
 */

/* Error codes */
typedef enum {
    ERROR_SUCCESS = 0,
    
    /* General errors */
    ERROR_INVALID_ARGUMENT = -1,
    ERROR_OUT_OF_MEMORY = -2,
    ERROR_FILE_NOT_FOUND = -3,
    ERROR_FILE_IO = -4,
    ERROR_PERMISSION_DENIED = -5,
    
    /* SMOF format errors */
    ERROR_INVALID_MAGIC = -10,
    ERROR_UNSUPPORTED_VERSION = -11,
    ERROR_CORRUPT_HEADER = -12,
    ERROR_INVALID_SECTION = -13,
    ERROR_INVALID_SYMBOL = -14,
    ERROR_INVALID_RELOCATION = -15,
    
    /* Linker errors */
    ERROR_SYMBOL_NOT_FOUND = -20,
    ERROR_DUPLICATE_SYMBOL = -21,
    ERROR_CIRCULAR_DEPENDENCY = -22,
    ERROR_RELOCATION_FAILED = -23,
    ERROR_SECTION_ALIGNMENT = -24,
    ERROR_OUTPUT_TOO_LARGE = -25,
    
    /* Archiver errors */
    ERROR_ARCHIVE_CORRUPT = -30,
    ERROR_MEMBER_NOT_FOUND = -31,
    ERROR_COMPRESSION_FAILED = -32,
    ERROR_DECOMPRESSION_FAILED = -33,
    
    /* System errors */
    ERROR_SYSTEM_LIMIT = -40,
    ERROR_INTERNAL = -99
} error_code_t;

/* Error severity levels */
typedef enum {
    ERROR_SEVERITY_INFO = 0,
    ERROR_SEVERITY_WARNING = 1,
    ERROR_SEVERITY_ERROR = 2,
    ERROR_SEVERITY_FATAL = 3
} error_severity_t;

/* Error context structure */
typedef struct error_context {
    error_code_t code;
    error_severity_t severity;
    const char* message;
    const char* file;
    int line;
    const char* function;
} error_context_t;

/* Error callback function type */
typedef void (*error_callback_t)(const error_context_t* context);

/* Error handling functions */
void error_set_callback(error_callback_t callback);
error_callback_t error_get_callback(void);

void error_report(error_code_t code, error_severity_t severity,
                 const char* file, int line, const char* function,
                 const char* message);

const char* error_get_string(error_code_t code);
bool error_is_fatal(error_code_t code);

/* Error macros for convenient reporting */
#define ERROR_REPORT_INFO(code, msg) \
    error_report(code, ERROR_SEVERITY_INFO, __FILE__, __LINE__, __func__, msg)

#define ERROR_REPORT_WARNING(code, msg) \
    error_report(code, ERROR_SEVERITY_WARNING, __FILE__, __LINE__, __func__, msg)

#define ERROR_REPORT_ERROR(code, msg) \
    error_report(code, ERROR_SEVERITY_ERROR, __FILE__, __LINE__, __func__, msg)

#define ERROR_REPORT_FATAL(code, msg) \
    error_report(code, ERROR_SEVERITY_FATAL, __FILE__, __LINE__, __func__, msg)

/* Assertion-like macros for error checking */
#define ERROR_CHECK(condition, code, msg) \
    do { \
        if (!(condition)) { \
            ERROR_REPORT_ERROR(code, msg); \
            return code; \
        } \
    } while (0)

#define ERROR_CHECK_NULL(ptr, msg) \
    ERROR_CHECK((ptr) != NULL, ERROR_INVALID_ARGUMENT, msg)

#define ERROR_CHECK_BOUNDS(value, min, max, msg) \
    ERROR_CHECK((value) >= (min) && (value) <= (max), ERROR_INVALID_ARGUMENT, msg)

/* C99 inline utility functions */
static inline bool error_is_success(error_code_t code) {
    return code == ERROR_SUCCESS;
}

static inline bool error_is_failure(error_code_t code) {
    return code != ERROR_SUCCESS;
}

static inline bool error_is_warning(error_code_t code) {
    return code < ERROR_SUCCESS && code > ERROR_INTERNAL;
}

/* Error message formatting */
#define ERROR_MSG_MAX_LENGTH 256

int error_format_message(char* buffer, size_t size, const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif /* ERROR_H_INCLUDED */
