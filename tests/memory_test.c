#include "memory.h"
#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Simple test for memory tracking functionality
int main(void) {
    // Initialize logger
    logger_init();
    logger_set_console_level(PEVI_LOG_DEBUG);
    
    // Initialize memory tracking
    memory_init();
    
    printf("Running memory allocation tests...\n");
    
    // Test basic allocation and free
    void *ptr1 = MALLOC(100);
    assert(ptr1 != NULL && "MALLOC should return non-NULL pointer");
    
    // Test calloc
    int *numbers = CALLOC(10, sizeof(int));
    assert(numbers != NULL && "CALLOC should return non-NULL pointer");
    
    // Verify calloc zeroes memory
    for (int i = 0; i < 10; i++) {
        assert(numbers[i] == 0 && "CALLOC should zero memory");
    }
    
    // Test realloc
    numbers = REALLOC(numbers, 20 * sizeof(int));
    assert(numbers != NULL && "REALLOC should return non-NULL pointer");
    
    // Test strdup
    const char *test_string = "Hello, memory test!";
    char *string_copy = STRDUP(test_string);
    assert(string_copy != NULL && "STRDUP should return non-NULL pointer");
    assert(strcmp(string_copy, test_string) == 0 && "STRDUP should copy string correctly");
    
    // Free all allocations
    FREE(ptr1);
    FREE(numbers);
    FREE(string_copy);
    
    // Check for leaks
    memory_print_leaks();
    
    // Test intentional leak
    void *leak_ptr = MALLOC(42);
    printf("Intentionally leaking 42 bytes at %p\n", leak_ptr);
    
    // Print leaks again - should show one leak
    memory_print_leaks();
    
    // Cleanup
    memory_cleanup();
    logger_cleanup();
    
    printf("Memory tests completed successfully!\n");
    return 0;
}
