/* src/common/error.c */
#include "error.h"
#include <stdio.h>
#include <stdarg.h>

/**
 * @file error.c
 * @brief Error handling implementation
 * @details C99 compliant error reporting and handling
 */

/* Forward declarations */
static const char* error_code_to_string(error_code_t code);

/* Global error callback */
static error_callback_t global_error_callback = NULL;

/* Error code to string mapping */
static const char* error_code_to_string(error_code_t code) {
    switch (code) {
        case ERROR_SUCCESS: return "Success";
        case ERROR_INVALID_ARGUMENT: return "Invalid argument";
        case ERROR_OUT_OF_MEMORY: return "Out of memory";
        case ERROR_FILE_NOT_FOUND: return "File not found";
        case ERROR_FILE_IO: return "File I/O error";
        case ERROR_PERMISSION_DENIED: return "Permission denied";
        case ERROR_INVALID_MAGIC: return "Invalid magic number";
        case ERROR_UNSUPPORTED_VERSION: return "Unsupported version";
        case ERROR_CORRUPT_HEADER: return "Corrupt header";
        case ERROR_INVALID_SECTION: return "Invalid section";
        case ERROR_INVALID_SYMBOL: return "Invalid symbol";
        case ERROR_INVALID_RELOCATION: return "Invalid relocation";
        case ERROR_SYMBOL_NOT_FOUND: return "Symbol not found";
        case ERROR_DUPLICATE_SYMBOL: return "Duplicate symbol";
        case ERROR_CIRCULAR_DEPENDENCY: return "Circular dependency";
        case ERROR_RELOCATION_FAILED: return "Relocation failed";
        case ERROR_SECTION_ALIGNMENT: return "Section alignment error";
        case ERROR_OUTPUT_TOO_LARGE: return "Output too large";
        case ERROR_ARCHIVE_CORRUPT: return "Archive corrupt";
        case ERROR_MEMBER_NOT_FOUND: return "Member not found";
        case ERROR_COMPRESSION_FAILED: return "Compression failed";
        case ERROR_DECOMPRESSION_FAILED: return "Decompression failed";
        case ERROR_SYSTEM_LIMIT: return "System limit exceeded";
        case ERROR_INTERNAL: return "Internal error";
        default: return "Unknown error";
    }
}

void error_set_callback(error_callback_t callback) {
    global_error_callback = callback;
}

error_callback_t error_get_callback(void) {
    return global_error_callback;
}

void error_report(error_code_t code, error_severity_t severity,
                 const char* file, int line, const char* function,
                 const char* message) {
    if (global_error_callback != NULL) {
        error_context_t context = {
            .code = code,
            .severity = severity,
            .message = message ? message : error_get_string(code),
            .file = file,
            .line = line,
            .function = function
        };
        
        global_error_callback(&context);
    }
}

const char* error_get_string(error_code_t code) {
    return error_code_to_string(code);
}

bool error_is_fatal(error_code_t code) {
    return code == ERROR_OUT_OF_MEMORY || 
           code == ERROR_INTERNAL ||
           code <= ERROR_INTERNAL;
}

int error_format_message(char* buffer, size_t size, const char* format, ...) {
    va_list args;
    int result;
    
    if (buffer == NULL || size == 0 || format == NULL) {
        return -1;
    }
    
    va_start(args, format);
    result = vsnprintf(buffer, size, format, args);
    va_end(args);
    
    return result;
}
