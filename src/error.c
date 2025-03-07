#include "error.h"
#include <stdlib.h>
#include <time.h>

// Global error state
ErrorContext_t g_last_error = {0};
bool g_error_occurred = false;

// Initialize error handling
void error_init(void) {
    ERROR_CLEAR();
}

// Log an error to stderr and optionally to a log file
void error_log(const ErrorContext_t *error) {
    if (!error) return;
    
    // Get current time
    time_t now = time(NULL);
    char time_str[26] = {0};
    
    // Format time string
    struct tm *tm_info = localtime(&now);
    strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Print to stderr
    fprintf(stderr, "[%s] %s: %s in %s:%d (%s)\n",
            time_str,
            error_level_string(error->level),
            error->message ? error->message : "Unknown error",
            error->file ? error->file : "unknown",
            error->line,
            error->function ? error->function : "unknown");
    
    // TODO: Optionally log to file if needed
}

// Handle fatal errors
void error_handle_fatal(const ErrorContext_t *error) {
    fprintf(stderr, "FATAL ERROR: Program will exit.\n");
    // Perform any cleanup needed before exit
    exit(EXIT_FAILURE);
}

// Convert error code to string
const char *error_code_string(ErrorCode_t code) {
    switch (code) {
        case ERROR_NONE: return "No error";
        case ERROR_FILE_NOT_FOUND: return "File not found";
        case ERROR_FILE_ACCESS: return "File access error";
        case ERROR_MEMORY_ALLOCATION: return "Memory allocation failed";
        case ERROR_SHADER_LOAD: return "Shader loading failed";
        case ERROR_FONT_LOAD: return "Font loading failed";
        case ERROR_BUFFER_OPERATION: return "Buffer operation failed";
        case ERROR_INVALID_PARAMETER: return "Invalid parameter";
        case ERROR_UNKNOWN: return "Unknown error";
        default: return "Undefined error";
    }
}

// Convert error level to string
const char *error_level_string(ErrorLevel_t level) {
    switch (level) {
        case ERROR_INFO: return "INFO";
        case ERROR_WARNING: return "WARNING";
        case ERROR_ERROR: return "ERROR";
        case ERROR_FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}
