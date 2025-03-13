#include "buffer.h"
#include "logger.h"
#include "memory.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Test for buffer functionality
int main(void) {
    // Initialize logger
    logger_init();
    logger_set_console_level(PEVI_LOG_DEBUG);
    
    // Initialize memory tracking
    memory_init();
    
    printf("Running buffer tests...\n");
    
    // Create a test file
    const char *test_filename = "buffer_test.txt";
    const char *test_content = "Line 1\nLine 2\nLine 3\n";
    
    FILE *test_file = fopen(test_filename, "w");
    assert(test_file != NULL && "Failed to create test file");
    fprintf(test_file, "%s", test_content);
    fclose(test_file);
    
    // Test opening a buffer
    Buffer_t *buffer = buffer_open((void*)test_filename);
    assert(buffer != NULL && "buffer_open should return non-NULL");
    assert(buffer->path != NULL && "Buffer path should be set");
    assert(strcmp(buffer->path, test_filename) == 0 && "Buffer path should match filename");
    assert(buffer->type == PEVI_BUF_FILE && "Buffer type should be PEVI_BUF_FILE");
    
    // Test buffer content
    // Verify the buffer contains the expected content by checking each line
    for (size_t line_no = 1; line_no <= 3; line_no++) {
        struct lr_cell *line = lr_owner_find(&buffer->lr, lr_owner(line_no));
        assert(line != NULL && "Line should exist in buffer");
        
        struct lr_cell *cell_head = lr_owner_head(&buffer->lr, line);
        struct lr_cell *cell_tail = lr_owner_tail(line);
        
        // Build a string from the line's cells
        char line_content[256] = {0};
        int pos = 0;
        
        struct lr_cell *needle = cell_head;
        do {
            line_content[pos++] = needle->data;
            needle = needle->next;
        } while (needle != cell_tail->next);
        
        line_content[pos] = '\0';
        
        // Check the line content
        char expected[256];
        sprintf(expected, "Line %zu", line_no);
        assert(strncmp(line_content, expected, strlen(expected)) == 0 && 
               "Line content should match expected");
    }
    
    // Test modifying the buffer
    // Insert text at the beginning of line 2
    struct lr_cell *line2 = lr_owner_find(&buffer->lr, lr_owner(2));
    assert(line2 != NULL && "Line 2 should exist");
    
    const char *insert_text = "Modified ";
    for (int i = strlen(insert_text) - 1; i >= 0; i--) {
        lr_result_t result = lr_insert(&buffer->lr, insert_text[i], 2, 1);
        assert(result == LR_SUCCESS && "Insert operation should succeed");
    }
    
    // Save the modified buffer to a new file
    const char *modified_filename = "buffer_test_modified.txt";
    bool save_result = buffer_save(buffer, (char*)modified_filename);
    assert(save_result && "buffer_save should return true");
    
    // Free the buffer
    buffer_free(buffer);
    
    // Verify the modified file content
    FILE *modified_file = fopen(modified_filename, "r");
    assert(modified_file != NULL && "Modified file should exist");
    
    char file_content[1024] = {0};
    size_t bytes_read = fread(file_content, 1, sizeof(file_content) - 1, modified_file);
    file_content[bytes_read] = '\0';
    fclose(modified_file);
    
    // Check that the modified content is correct
    assert(strstr(file_content, "Modified Line 2") != NULL && 
           "Modified file should contain the inserted text");
    
    // Clean up test files
    remove(test_filename);
    remove(modified_filename);
    
    // Check for memory leaks
    memory_print_leaks();
    
    // Cleanup
    memory_cleanup();
    logger_cleanup();
    
    printf("Buffer tests completed successfully!\n");
    return 0;
}
