#pragma once
#include "phantom.h"
#include "memory.h"
#include "error.h"
#include "logger.h"

// Node in the phantom linked list
typedef struct PhantomNode {
    Phantom_t *phantom;           // Pointer to the phantom
    struct PhantomNode *next;     // Pointer to the next node
    struct PhantomNode *prev;     // Pointer to the previous node
} PhantomNode_t;

// Phantom list structure
typedef struct {
    PhantomNode_t *head;          // First node in the list
    PhantomNode_t *tail;          // Last node in the list
    PhantomNode_t *active;        // Currently active node
    int count;                    // Number of phantoms in the list
    int next_id;                  // Next ID to assign to a phantom
} PhantomList_t;

// Initialize a new phantom list
PhantomList_t *phantom_list_create(void);

// Free the phantom list and all phantoms
void phantom_list_free(PhantomList_t *list);

// Add a phantom to the list
// Returns the node containing the phantom, or NULL on failure
PhantomNode_t *phantom_list_add(PhantomList_t *list, Phantom_t *phantom);

// Create and add a new phantom with default settings
// Returns the node containing the phantom, or NULL on failure
PhantomNode_t *phantom_list_create_phantom(PhantomList_t *list, const char *filename);

// Remove a phantom from the list by node
// Returns true if successful, false otherwise
bool phantom_list_remove(PhantomList_t *list, PhantomNode_t *node);

// Remove a phantom from the list by ID
// Returns true if successful, false otherwise
bool phantom_list_remove_by_id(PhantomList_t *list, int id);

// Get a phantom node by ID
// Returns NULL if not found
PhantomNode_t *phantom_list_get_by_id(PhantomList_t *list, int id);

// Get the active phantom
// Returns NULL if no active phantom
Phantom_t *phantom_list_get_active(PhantomList_t *list);

// Set the active phantom by node
// Returns true if successful, false otherwise
bool phantom_list_set_active(PhantomList_t *list, PhantomNode_t *node);

// Set the active phantom by ID
// Returns true if successful, false otherwise
bool phantom_list_set_active_by_id(PhantomList_t *list, int id);

// Get the count of phantoms
int phantom_list_get_count(PhantomList_t *list);

// Draw all phantoms
void phantom_list_draw_all(PhantomList_t *list, Camera_t *camera, InputEvent_t *event);

// Move to the next phantom in the list and make it active
// Returns true if successful, false if there is no next phantom
bool phantom_list_next(PhantomList_t *list);

// Move to the previous phantom in the list and make it active
// Returns true if successful, false if there is no previous phantom
bool phantom_list_prev(PhantomList_t *list);
