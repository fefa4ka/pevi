#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

// Log levels
typedef enum {
    PEVI_LOG_TRACE,    // Detailed tracing information
    PEVI_LOG_DEBUG,    // Debugging information
    PEVI_LOG_INFO,     // General information
    PEVI_LOG_WARNING,  // Warning messages
    PEVI_LOG_ERROR,    // Error messages
    PEVI_LOG_FATAL     // Fatal errors
} LogLevel;

// Log configuration
typedef struct {
    LogLevel console_level;     // Minimum level to log to console
    LogLevel file_level;        // Minimum level to log to file
    bool use_colors;            // Whether to use colors in console output
    bool log_to_file;           // Whether to log to file
    char log_file_path[256];    // Path to log file
    FILE *log_file;             // Log file handle
} LogConfig;

// Initialize the logger
void logger_init(void);

// Set the minimum log level for console output
void logger_set_console_level(LogLevel level);

// Set the minimum log level for file output
void logger_set_file_level(LogLevel level);

// Enable or disable colored output
void logger_set_use_colors(bool use_colors);

// Enable or disable file logging
void logger_set_log_to_file(bool log_to_file, const char *file_path);

// Log a message
void logger_log(LogLevel level, const char *file, int line, const char *function, const char *format, ...);

// Cleanup logger resources
void logger_cleanup(void);

// Convenience macros for logging
#define LOG_TRACE(...) logger_log(PEVI_LOG_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_DEBUG(...) logger_log(PEVI_LOG_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_INFO(...) logger_log(PEVI_LOG_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARNING(...) logger_log(PEVI_LOG_WARNING, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(...) logger_log(PEVI_LOG_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_FATAL(...) logger_log(PEVI_LOG_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

// Get string representation of log level
const char *logger_level_string(LogLevel level);
