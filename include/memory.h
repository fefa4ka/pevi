#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include "error.h"

// Memory tracking structure
typedef struct {
    void *ptr;              // Pointer to allocated memory
    size_t size;            // Size of allocation
    const char *file;       // Source file where allocation occurred
    int line;               // Line number where allocation occurred
    const char *function;   // Function where allocation occurred
    bool freed;             // Whether this memory has been freed
} MemoryAllocation_t;

// Memory tracking functions
void memory_init(void);
void memory_cleanup(void);
void memory_print_leaks(void);

// Memory allocation wrappers
void *memory_malloc(size_t size, const char *file, int line, const char *function);
void *memory_calloc(size_t count, size_t size, const char *file, int line, const char *function);
void *memory_realloc(void *ptr, size_t size, const char *file, int line, const char *function);
char *memory_strdup(const char *str, const char *file, int line, const char *function);
void memory_free(void *ptr, const char *file, int line, const char *function);

// Macros for easy use
#define MALLOC(size) memory_malloc(size, __FILE__, __LINE__, __func__)
#define CALLOC(count, size) memory_calloc(count, size, __FILE__, __LINE__, __func__)
#define REALLOC(ptr, size) memory_realloc(ptr, size, __FILE__, __LINE__, __func__)
#define STRDUP(str) memory_strdup(str, __FILE__, __LINE__, __func__)
#define FREE(ptr) memory_free(ptr, __FILE__, __LINE__, __func__)
