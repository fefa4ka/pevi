#include "logger.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// ANSI color codes
#define COLOR_RESET   "\x1b[0m"
#define COLOR_GRAY    "\x1b[90m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_MAGENTA "\x1b[35m"

// Global logger configuration
static LogConfig g_log_config = {
    .console_level = PEVI_LOG_INFO,
    .file_level = PEVI_LOG_TRACE,
    .use_colors = true,
    .log_to_file = false,
    .log_file_path = "",
    .log_file = NULL
};

// Initialize the logger
void logger_init(void) {
    // Default configuration is already set in the static initialization
    // Just ensure the log file is closed
    if (g_log_config.log_file != NULL) {
        fclose(g_log_config.log_file);
        g_log_config.log_file = NULL;
    }
}

// Set the minimum log level for console output
void logger_set_console_level(LogLevel level) {
    g_log_config.console_level = level;
}

// Set the minimum log level for file output
void logger_set_file_level(LogLevel level) {
    g_log_config.file_level = level;
}

// Enable or disable colored output
void logger_set_use_colors(bool use_colors) {
    g_log_config.use_colors = use_colors;
}

// Enable or disable file logging
void logger_set_log_to_file(bool log_to_file, const char *file_path) {
    // Close existing log file if open
    if (g_log_config.log_file != NULL) {
        fclose(g_log_config.log_file);
        g_log_config.log_file = NULL;
    }
    
    g_log_config.log_to_file = log_to_file;
    
    if (log_to_file && file_path != NULL) {
        strncpy(g_log_config.log_file_path, file_path, sizeof(g_log_config.log_file_path) - 1);
        g_log_config.log_file_path[sizeof(g_log_config.log_file_path) - 1] = '\0';
        
        // Open the log file
        g_log_config.log_file = fopen(g_log_config.log_file_path, "a");
        if (g_log_config.log_file == NULL) {
            fprintf(stderr, "Failed to open log file: %s\n", g_log_config.log_file_path);
            g_log_config.log_to_file = false;
        }
    }
}

// Get color code for log level
static const char *get_level_color(LogLevel level) {
    switch (level) {
        case PEVI_LOG_TRACE: return COLOR_GRAY;
        case PEVI_LOG_DEBUG: return COLOR_GREEN;
        case PEVI_LOG_INFO: return COLOR_BLUE;
        case PEVI_LOG_WARNING: return COLOR_YELLOW;
        case PEVI_LOG_ERROR: return COLOR_RED;
        case PEVI_LOG_FATAL: return COLOR_MAGENTA;
        default: return COLOR_RESET;
    }
}

// Get string representation of log level
const char *logger_level_string(LogLevel level) {
    switch (level) {
        case PEVI_LOG_TRACE: return "TRACE";
        case PEVI_LOG_DEBUG: return "DEBUG";
        case PEVI_LOG_INFO: return "INFO";
        case PEVI_LOG_WARNING: return "WARNING";
        case PEVI_LOG_ERROR: return "ERROR";
        case PEVI_LOG_FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

// Log a message
void logger_log(LogLevel level, const char *file, int line, const char *function, const char *format, ...) {
    // Skip if log level is below minimum for both console and file
    if (level < g_log_config.console_level && 
        (level < g_log_config.file_level || !g_log_config.log_to_file)) {
        return;
    }
    
    // Get current time
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Format the message
    va_list args;
    va_start(args, format);
    char message[1024];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    // Extract filename from path
    const char *filename = file;
    const char *last_slash = strrchr(file, '/');
    if (last_slash != NULL) {
        filename = last_slash + 1;
    }
    
    // Log to console if level is high enough
    if (level >= g_log_config.console_level) {
        if (g_log_config.use_colors) {
            const char *color = get_level_color(level);
            fprintf(stderr, "%s[%s] %s%-7s%s %s:%d (%s): %s\n",
                    color, time_str, color, logger_level_string(level), COLOR_RESET,
                    filename, line, function, message);
        } else {
            fprintf(stderr, "[%s] %-7s %s:%d (%s): %s\n",
                    time_str, logger_level_string(level),
                    filename, line, function, message);
        }
    }
    
    // Log to file if enabled and level is high enough
    if (g_log_config.log_to_file && level >= g_log_config.file_level && g_log_config.log_file != NULL) {
        fprintf(g_log_config.log_file, "[%s] %-7s %s:%d (%s): %s\n",
                time_str, logger_level_string(level),
                file, line, function, message);
        fflush(g_log_config.log_file);
    }
}

// Cleanup logger resources
void logger_cleanup(void) {
    if (g_log_config.log_file != NULL) {
        fclose(g_log_config.log_file);
        g_log_config.log_file = NULL;
    }
}
