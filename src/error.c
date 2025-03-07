#include "error.h"
#include "logger.h"
#include <stdlib.h>

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
    
    // Map error level to log level
    LogLevel log_level;
    switch (error->level) {
        case ERROR_INFO:    log_level = PEVI_LOG_INFO;    break;
        case ERROR_WARNING: log_level = PEVI_LOG_WARNING; break;
        case ERROR_ERROR:   log_level = PEVI_LOG_ERROR;   break;
        case ERROR_FATAL:   log_level = PEVI_LOG_FATAL;   break;
        default:            log_level = PEVI_LOG_ERROR;   break;
    }
    
    // Log the error using our logger
    logger_log(log_level, error->file, error->line, error->function,
               "%s: %s", error_code_string(error->code), 
               error->message ? error->message : "Unknown error");
}

// Handle fatal errors
void error_handle_fatal(const ErrorContext_t *error) {
    LOG_FATAL("FATAL ERROR: Program will exit.");
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
