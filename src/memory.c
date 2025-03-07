#include "memory.h"
#include "logger.h"
#include <stdio.h>
#include <string.h>

#define MAX_ALLOCATIONS 1024

// Array to track memory allocations
static MemoryAllocation_t allocations[MAX_ALLOCATIONS];
static int allocation_count = 0;

// Initialize memory tracking
void memory_init(void) {
    allocation_count = 0;
    memset(allocations, 0, sizeof(allocations));
    LOG_DEBUG("Memory tracking initialized with capacity for %d allocations", MAX_ALLOCATIONS);
}

// Find allocation entry for a pointer
static int find_allocation(void *ptr) {
    for (int i = 0; i < allocation_count; i++) {
        if (allocations[i].ptr == ptr && !allocations[i].freed) {
            return i;
        }
    }
    return -1;
}

// Add a new allocation to tracking
static void track_allocation(void *ptr, size_t size, const char *file, int line, const char *function) {
    if (!ptr) return;
    
    if (allocation_count >= MAX_ALLOCATIONS) {
        LOG_WARNING("Memory tracking array full, cannot track more allocations");
        return;
    }
    
    allocations[allocation_count].ptr = ptr;
    allocations[allocation_count].size = size;
    allocations[allocation_count].file = file;
    allocations[allocation_count].line = line;
    allocations[allocation_count].function = function;
    allocations[allocation_count].freed = false;
    
    LOG_TRACE("Allocated %zu bytes at %p in %s:%d (%s)", 
              size, ptr, file, line, function);
    
    allocation_count++;
}

// Mark an allocation as freed
static void mark_freed(void *ptr, const char *file, int line, const char *function) {
    int index = find_allocation(ptr);
    
    if (index >= 0) {
        allocations[index].freed = true;
        LOG_TRACE("Freed %zu bytes at %p in %s:%d (%s)", 
                  allocations[index].size, ptr, file, line, function);
    } else if (ptr != NULL) {
        LOG_WARNING("Attempt to free untracked pointer %p at %s:%d (%s)", 
                    ptr, file, line, function);
    }
}

// Malloc wrapper with tracking
void *memory_malloc(size_t size, const char *file, int line, const char *function) {
    void *ptr = malloc(size);
    
    if (!ptr && size > 0) {
        LOG_ERROR("Failed to allocate %zu bytes in %s:%d (%s)", 
                  size, file, line, function);
        ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to allocate memory");
        return NULL;
    }
    
    track_allocation(ptr, size, file, line, function);
    return ptr;
}

// Calloc wrapper with tracking
void *memory_calloc(size_t count, size_t size, const char *file, int line, const char *function) {
    void *ptr = calloc(count, size);
    
    if (!ptr && count > 0 && size > 0) {
        LOG_ERROR("Failed to allocate %zu bytes (%zu x %zu) in %s:%d (%s)", 
                  count * size, count, size, file, line, function);
        ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to allocate memory");
        return NULL;
    }
    
    track_allocation(ptr, count * size, file, line, function);
    return ptr;
}

// Realloc wrapper with tracking
void *memory_realloc(void *ptr, size_t size, const char *file, int line, const char *function) {
    int index = find_allocation(ptr);
    
    void *new_ptr = realloc(ptr, size);
    
    if (!new_ptr && size > 0) {
        LOG_ERROR("Failed to reallocate %zu bytes at %p in %s:%d (%s)", 
                  size, ptr, file, line, function);
        ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to reallocate memory");
        return NULL;
    }
    
    if (index >= 0) {
        // Mark old allocation as freed
        allocations[index].freed = true;
        LOG_TRACE("Reallocated %zu bytes at %p to %zu bytes at %p in %s:%d (%s)", 
                  allocations[index].size, ptr, size, new_ptr, file, line, function);
    } else {
        LOG_TRACE("Reallocated untracked pointer %p to %zu bytes at %p in %s:%d (%s)", 
                  ptr, size, new_ptr, file, line, function);
    }
    
    track_allocation(new_ptr, size, file, line, function);
    return new_ptr;
}

// Strdup wrapper with tracking
char *memory_strdup(const char *str, const char *file, int line, const char *function) {
    if (!str) return NULL;
    
    size_t len = strlen(str) + 1;
    char *new_str = memory_malloc(len, file, line, function);
    
    if (new_str) {
        memcpy(new_str, str, len);
    }
    
    return new_str;
}

// Free wrapper with tracking
void memory_free(void *ptr, const char *file, int line, const char *function) {
    if (!ptr) return;
    
    mark_freed(ptr, file, line, function);
    free(ptr);
}

// Print memory leaks
void memory_print_leaks(void) {
    int leak_count = 0;
    size_t total_leaked = 0;
    
    LOG_INFO("=== Memory Leak Report ===");
    
    for (int i = 0; i < allocation_count; i++) {
        if (!allocations[i].freed) {
            LOG_WARNING("LEAK: %zu bytes at %p allocated in %s:%d (%s)",
                        allocations[i].size, allocations[i].ptr,
                        allocations[i].file, allocations[i].line,
                        allocations[i].function);
            
            leak_count++;
            total_leaked += allocations[i].size;
        }
    }
    
    if (leak_count == 0) {
        LOG_INFO("No memory leaks detected.");
    } else {
        LOG_WARNING("Total: %d leaks, %zu bytes", leak_count, total_leaked);
    }
    
    LOG_INFO("=========================");
}

// Clean up all remaining allocations
void memory_cleanup(void) {
    for (int i = 0; i < allocation_count; i++) {
        if (!allocations[i].freed && allocations[i].ptr) {
            LOG_WARNING("Freeing leaked memory at %p (%zu bytes) from %s:%d",
                        allocations[i].ptr, allocations[i].size,
                        allocations[i].file, allocations[i].line);
            free(allocations[i].ptr);
            allocations[i].freed = true;
        }
    }
    
    LOG_DEBUG("Memory cleanup complete, freed %d allocations", allocation_count);
    allocation_count = 0;
}
