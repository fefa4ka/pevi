#include "phantom_list.h"
#include "buffer.h"
#include "logger.h"
#include "memory.h"
#include "pevi.h"
#include "error.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>


Pevi_t pevi = {PEVI_MODE_FREE, true};

Command_t commands[] = {
    {0} // End marker
};


// Helper function to create a test file
static bool create_test_file(const char *filename, const char *content) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Failed to create test file: %s\n", filename);
        return false;
    }
    
    fprintf(file, "%s", content);
    fclose(file);
    return true;
}

// Test phantom list creation and basic operations
static void test_phantom_list_creation(void) {
    printf("Testing phantom list creation...\n");
    
    // Create a phantom list
    PhantomList_t *list = phantom_list_create();
    assert(list != NULL && "phantom_list_create should return non-NULL");
    assert(list->count == 0 && "New list should have count 0");
    assert(list->head == NULL && "New list should have NULL head");
    assert(list->tail == NULL && "New list should have NULL tail");
    assert(list->active == NULL && "New list should have NULL active");
    
    // Free the list
    phantom_list_free(list);
    printf("Phantom list creation test passed\n");
}

// Test phantom creation and addition to list
static void test_phantom_creation(void) {
    printf("Testing phantom creation...\n");
    
    // Create a test file
    const char *test_filename = "phantom_test_file.txt";
    create_test_file(test_filename, "Test content\nfor phantom\nmanagement test");
    
    // Create a phantom list
    PhantomList_t *list = phantom_list_create();
    
    // Create a phantom without a file
    PhantomNode_t *node1 = phantom_list_create_phantom(list, NULL);
    assert(node1 != NULL && "phantom_list_create_phantom should return non-NULL node");
    assert(node1->phantom != NULL && "Node should have non-NULL phantom");
    assert(list->count == 1 && "List should have count 1 after adding phantom");
    assert(list->active == node1 && "First phantom should be active");
    
    // Create a phantom with a file
    PhantomNode_t *node2 = phantom_list_create_phantom(list, test_filename);
    assert(node2 != NULL && "phantom_list_create_phantom should return non-NULL node");
    assert(node2->phantom != NULL && "Node should have non-NULL phantom");
    assert(node2->phantom->buffer != NULL && "Phantom should have non-NULL buffer");
    assert(list->count == 2 && "List should have count 2 after adding second phantom");
    
    // Check that the phantoms have different IDs
    assert(node1->phantom->id != node2->phantom->id && "Phantoms should have different IDs");
    
    // Free the list (which also frees the phantoms)
    phantom_list_free(list);
    
    // Clean up test file
    remove(test_filename);
    
    printf("Phantom creation test passed\n");
}

// Test phantom selection and navigation
static void test_phantom_selection(void) {
    printf("Testing phantom selection and navigation...\n");
    
    // Create a phantom list
    PhantomList_t *list = phantom_list_create();
    
    // Create three phantoms
    PhantomNode_t *node1 = phantom_list_create_phantom(list, NULL);
    PhantomNode_t *node2 = phantom_list_create_phantom(list, NULL);
    PhantomNode_t *node3 = phantom_list_create_phantom(list, NULL);
    
    // Check initial active phantom (should be the first one created)
    assert(list->active == node1 && "First phantom should be active initially");
    
    // Test moving to next phantom
    bool result = phantom_list_next(list);
    assert(result && "phantom_list_next should return true");
    assert(list->active == node2 && "Active phantom should be the second one");
    
    // Test moving to next phantom again
    result = phantom_list_next(list);
    assert(result && "phantom_list_next should return true");
    assert(list->active == node3 && "Active phantom should be the third one");
    
    // Test moving to next phantom when at the end (should fail)
    result = phantom_list_next(list);
    assert(!result && "phantom_list_next should return false at end of list");
    assert(list->active == node3 && "Active phantom should still be the third one");
    
    // Test moving to previous phantom
    result = phantom_list_prev(list);
    assert(result && "phantom_list_prev should return true");
    assert(list->active == node2 && "Active phantom should be the second one");
    
    // Test setting active phantom by ID
    result = phantom_list_set_active_by_id(list, node3->phantom->id);
    assert(result && "phantom_list_set_active_by_id should return true");
    assert(list->active == node3 && "Active phantom should be the third one");
    
    // Test setting active phantom by node
    result = phantom_list_set_active(list, node1);
    assert(result && "phantom_list_set_active should return true");
    assert(list->active == node1 && "Active phantom should be the first one");
    
    // Free the list
    phantom_list_free(list);
    
    printf("Phantom selection and navigation test passed\n");
}

// Test phantom removal
static void test_phantom_removal(void) {
    printf("Testing phantom removal...\n");
    
    // Create a phantom list
    PhantomList_t *list = phantom_list_create();
    
    // Create three phantoms
    PhantomNode_t *node1 = phantom_list_create_phantom(list, NULL);
    PhantomNode_t *node2 = phantom_list_create_phantom(list, NULL);
    PhantomNode_t *node3 = phantom_list_create_phantom(list, NULL);
    
    // Store IDs for later reference
    int id1 = node1->phantom->id;
    int id2 = node2->phantom->id;
    int id3 = node3->phantom->id;
    
    // Remove the middle phantom by node
    bool result = phantom_list_remove(list, node2);
    assert(result && "phantom_list_remove should return true");
    assert(list->count == 2 && "List should have count 2 after removal");
    
    // Check that node1 and node3 are still connected properly
    assert(node1->next == node3 && "node1->next should be node3 after removing node2");
    assert(node3->prev == node1 && "node3->prev should be node1 after removing node2");
    
    // Try to get the removed phantom by ID (should fail)
    PhantomNode_t *not_found = phantom_list_get_by_id(list, id2);
    assert(not_found == NULL && "phantom_list_get_by_id should return NULL for removed phantom");
    
    // Remove the first phantom by ID
    result = phantom_list_remove_by_id(list, id1);
    assert(result && "phantom_list_remove_by_id should return true");
    assert(list->count == 1 && "List should have count 1 after second removal");
    assert(list->head == node3 && "List head should be node3 after removing node1");
    assert(list->tail == node3 && "List tail should be node3 after removing node1");
    
    // Check that the active phantom is updated correctly
    assert(list->active == node3 && "Active phantom should be node3 after removals");
    
    // Free the list
    phantom_list_free(list);
    
    printf("Phantom removal test passed\n");
}

int main(void) {
    // Initialize logger
    logger_init();
    logger_set_console_level(PEVI_LOG_INFO);
    
    // Initialize memory tracking
    memory_init();
    
    // Initialize error handling
    error_init();
    
    printf("Running phantom management tests...\n");
    
    // Run tests
    test_phantom_list_creation();
    test_phantom_creation();
    test_phantom_selection();
    test_phantom_removal();
    
    // Check for memory leaks
    memory_print_leaks();
    
    // Cleanup
    memory_cleanup();
    logger_cleanup();
    
    printf("All phantom management tests passed!\n");
    return 0;
}
