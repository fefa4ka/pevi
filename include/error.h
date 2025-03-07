#pragma once
#include <stdbool.h>
#include <stdio.h>

// Error severity levels
typedef enum {
    ERROR_INFO,     // Informational message, not an error
    ERROR_WARNING,  // Warning, operation can continue
    ERROR_ERROR,    // Error, operation failed but program can continue
    ERROR_FATAL     // Fatal error, program should terminate
} ErrorLevel_t;

// Error codes
typedef enum {
    ERROR_NONE,                 // No error
    ERROR_FILE_NOT_FOUND,       // File not found
    ERROR_FILE_ACCESS,          // File access error
    ERROR_MEMORY_ALLOCATION,    // Memory allocation failed
    ERROR_SHADER_LOAD,          // Shader loading failed
    ERROR_FONT_LOAD,            // Font loading failed
    ERROR_BUFFER_OPERATION,     // Buffer operation failed
    ERROR_INVALID_PARAMETER,    // Invalid parameter
    ERROR_UNKNOWN               // Unknown error
} ErrorCode_t;

// Error context structure
typedef struct {
    ErrorCode_t code;           // Error code
    ErrorLevel_t level;         // Error severity level
    const char *message;        // Error message
    const char *file;           // Source file where error occurred
    int line;                   // Line number where error occurred
    const char *function;       // Function where error occurred
} ErrorContext_t;

// Global error state
extern ErrorContext_t g_last_error;
extern bool g_error_occurred;

// Macros for error handling
#define ERROR_CLEAR() do { \
    g_error_occurred = false; \
    g_last_error.code = ERROR_NONE; \
    g_last_error.level = ERROR_INFO; \
    g_last_error.message = NULL; \
    g_last_error.file = NULL; \
    g_last_error.line = 0; \
    g_last_error.function = NULL; \
} while(0)

#define ERROR_SET(code_value, level_value, message_value) do { \
    g_error_occurred = true; \
    g_last_error.code = (code_value); \
    g_last_error.level = (level_value); \
    g_last_error.message = (message_value); \
    g_last_error.file = __FILE__; \
    g_last_error.line = __LINE__; \
    g_last_error.function = __func__; \
    error_log(&g_last_error); \
    if ((level_value) == ERROR_FATAL) { \
        error_handle_fatal(&g_last_error); \
    } \
} while(0)

#define ERROR_CHECK() (g_error_occurred)
#define ERROR_GET() (g_last_error)

// Function prototypes
void error_init(void);
void error_log(const ErrorContext_t *error);
void error_handle_fatal(const ErrorContext_t *error);
const char *error_code_string(ErrorCode_t code);
const char *error_level_string(ErrorLevel_t level);
