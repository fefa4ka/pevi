#pragma once

// This file defines constants for error codes and levels
// to avoid direct use of enum values in the code

// Error severity levels
#define ERROR_INFO      ERROR_INFO
#define ERROR_WARNING   ERROR_WARNING
#define ERROR_ERROR     ERROR_ERROR
#define ERROR_FATAL     ERROR_FATAL

// Error codes
#define ERROR_NONE                ERROR_NONE
#define ERROR_FILE_NOT_FOUND      ERROR_FILE_NOT_FOUND
#define ERROR_FILE_ACCESS         ERROR_FILE_ACCESS
#define ERROR_MEMORY_ALLOCATION   ERROR_MEMORY_ALLOCATION
#define ERROR_SHADER_LOAD         ERROR_SHADER_LOAD
#define ERROR_FONT_LOAD           ERROR_FONT_LOAD
#define ERROR_BUFFER_OPERATION    ERROR_BUFFER_OPERATION
#define ERROR_INVALID_PARAMETER   ERROR_INVALID_PARAMETER
#define ERROR_UNKNOWN             ERROR_UNKNOWN
