#include "phantom_list.h"

// Initialize a new phantom list
PhantomList_t *phantom_list_create(void) {
    LOG_DEBUG("Creating phantom list");
    
    PhantomList_t *list = MALLOC(sizeof(PhantomList_t));
    if (!list) {
        ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to allocate phantom list");
        return NULL;
    }
    
    list->head = NULL;
    list->tail = NULL;
    list->active = NULL;
    list->count = 0;
    list->next_id = 1;
    
    LOG_INFO("Phantom list created");
    return list;
}

// Free the phantom list and all phantoms
void phantom_list_free(PhantomList_t *list) {
    if (!list) {
        LOG_WARNING("Attempt to free NULL phantom list");
        return;
    }
    
    LOG_INFO("Freeing phantom list with %d phantoms", list->count);
    
    // Free all phantoms and nodes
    PhantomNode_t *current = list->head;
    while (current) {
        PhantomNode_t *next = current->next;
        
        // Free the phantom
        if (current->phantom) {
            phantom_free(current->phantom);
        }
        
        // Free the node
        FREE(current);
        
        current = next;
    }
    
    // Free the list itself
    FREE(list);
    
    LOG_DEBUG("Phantom list freed");
}

// Create a new node for a phantom
static PhantomNode_t *create_node(Phantom_t *phantom) {
    PhantomNode_t *node = MALLOC(sizeof(PhantomNode_t));
    if (!node) {
        ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to allocate phantom node");
        return NULL;
    }
    
    node->phantom = phantom;
    node->next = NULL;
    node->prev = NULL;
    
    return node;
}

// Add a phantom to the list
PhantomNode_t *phantom_list_add(PhantomList_t *list, Phantom_t *phantom) {
    if (!list || !phantom) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "NULL parameter in phantom_list_add");
        return NULL;
    }
    
    // Set phantom ID if not already set
    if (phantom->id == 0) {
        phantom->id = list->next_id++;
    }
    
    // Create a new node
    PhantomNode_t *node = create_node(phantom);
    if (!node) {
        return NULL;
    }
    
    // Add to the end of the list
    if (list->tail) {
        // List is not empty
        list->tail->next = node;
        node->prev = list->tail;
        list->tail = node;
    } else {
        // List is empty
        list->head = node;
        list->tail = node;
    }
    
    // Increment count
    list->count++;
    
    // If this is the first phantom, make it active
    if (list->count == 1) {
        list->active = node;
    }
    
    LOG_INFO("Added phantom with ID %d to list", phantom->id);
    return node;
}

// Create and add a new phantom with default settings
PhantomNode_t *phantom_list_create_phantom(PhantomList_t *list, const char *filename) {
    if (!list) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "NULL list in phantom_list_create_phantom");
        return NULL;
    }
    
    LOG_INFO("Creating new phantom for file: %s", filename ? filename : "NULL");
    
    // Create a new phantom
    Phantom_t *phantom = phantom_create();
    if (!phantom) {
        ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to create phantom");
        return NULL;
    }
    
    // Set default values
    phantom->font.font_size = 16;
    phantom->font.spacing = 0.6;
    phantom->font.line_spacing = 0.6;
    phantom->line_from = 1;
    phantom->line_to = 3;
    
    // Open buffer if filename provided
    if (filename) {
        phantom->buffer = buffer_open((void*)filename);
        if (!phantom->buffer) {
            ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_ERROR, "Failed to open buffer");
            phantom_free(phantom);
            return NULL;
        }
    }
    
    // Add phantom to list
    PhantomNode_t *node = phantom_list_add(list, phantom);
    if (!node) {
        phantom_free(phantom);
        return NULL;
    }
    
    return node;
}

// Remove a phantom from the list by node
bool phantom_list_remove(PhantomList_t *list, PhantomNode_t *node) {
    if (!list || !node) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "NULL parameter in phantom_list_remove");
        return false;
    }
    
    LOG_INFO("Removing phantom node from list");
    
    // Update active node if needed
    if (list->active == node) {
        // Try to set active to next, or prev if no next, or NULL if neither
        if (node->next) {
            list->active = node->next;
        } else if (node->prev) {
            list->active = node->prev;
        } else {
            list->active = NULL;
        }
    }
    
    // Update head if needed
    if (list->head == node) {
        list->head = node->next;
    }
    
    // Update tail if needed
    if (list->tail == node) {
        list->tail = node->prev;
    }
    
    // Update adjacent nodes
    if (node->prev) {
        node->prev->next = node->next;
    }
    
    if (node->next) {
        node->next->prev = node->prev;
    }
    
    // Free the phantom
    if (node->phantom) {
        phantom_free(node->phantom);
    }
    
    // Free the node
    FREE(node);
    
    // Decrement count
    list->count--;
    
    LOG_INFO("Phantom removed, new count: %d", list->count);
    return true;
}

// Remove a phantom from the list by ID
bool phantom_list_remove_by_id(PhantomList_t *list, int id) {
    if (!list) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "NULL list in phantom_list_remove_by_id");
        return false;
    }
    
    // Find the node with the given ID
    PhantomNode_t *node = phantom_list_get_by_id(list, id);
    if (!node) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Phantom with ID not found");
        return false;
    }
    
    // Remove the node
    return phantom_list_remove(list, node);
}

// Get a phantom node by ID
PhantomNode_t *phantom_list_get_by_id(PhantomList_t *list, int id) {
    if (!list) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "NULL list in phantom_list_get_by_id");
        return NULL;
    }
    
    PhantomNode_t *current = list->head;
    while (current) {
        if (current->phantom && current->phantom->id == id) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// Get the active phantom
Phantom_t *phantom_list_get_active(PhantomList_t *list) {
    if (!list) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "NULL list in phantom_list_get_active");
        return NULL;
    }
    
    if (!list->active) {
        return NULL;
    }
    
    return list->active->phantom;
}

// Set the active phantom by node
bool phantom_list_set_active(PhantomList_t *list, PhantomNode_t *node) {
    if (!list) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "NULL list in phantom_list_set_active");
        return false;
    }
    
    if (!node) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "NULL node in phantom_list_set_active");
        return false;
    }
    
    // Verify the node is in the list
    PhantomNode_t *current = list->head;
    bool found = false;
    
    while (current) {
        if (current == node) {
            found = true;
            break;
        }
        current = current->next;
    }
    
    if (!found) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Node not found in list");
        return false;
    }
    
    // Clear selection on all phantoms except the new active one
    current = list->head;
    while (current) {
        if (current != node && current->phantom) {
            current->phantom->is_selected = false;
        }
        current = current->next;
    }
    
    // Set the new active phantom and ensure it's selected
    list->active = node;
    if (node->phantom) {
        node->phantom->is_selected = true;
    }
    
    LOG_INFO("Set active phantom to ID %d", node->phantom->id);
    return true;
}

// Set the active phantom by ID
bool phantom_list_set_active_by_id(PhantomList_t *list, int id) {
    if (!list) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "NULL list in phantom_list_set_active_by_id");
        return false;
    }
    
    PhantomNode_t *node = phantom_list_get_by_id(list, id);
    if (!node) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Phantom with ID not found");
        return false;
    }
    
    return phantom_list_set_active(list, node);
}

// Get the count of phantoms
int phantom_list_get_count(PhantomList_t *list) {
    if (!list) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "NULL list in phantom_list_get_count");
        return 0;
    }
    
    return list->count;
}

// Draw all phantoms
void phantom_list_draw_all(PhantomList_t *list, Camera_t *camera, InputEvent_t *event) {
    if (!list || !camera || !event) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "NULL parameter in phantom_list_draw_all");
        return;
    }
    
    // Draw all phantoms
    PhantomNode_t *current = list->head;
    while (current) {
        if (current->phantom) {
            // Create a copy of the event for each phantom
            InputEvent_t phantom_event = *event;
            
            if (!phantom_draw_on_plane(current->phantom, camera, &phantom_event)) {
                LOG_WARNING("Failed to draw phantom with ID %d", current->phantom->id);
            }
            
            // If this phantom handled the event, update the original event
            if (phantom_event.source_type != INPUT_SOURCE_NONE) {
                *event = phantom_event;
            }
        }
        
        current = current->next;
    }
}

// Move to the next phantom in the list and make it active
bool phantom_list_next(PhantomList_t *list) {
    if (!list) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "NULL list in phantom_list_next");
        return false;
    }
    
    if (!list->active || !list->active->next) {
        return false;
    }
    
    list->active = list->active->next;
    LOG_INFO("Moved to next phantom, ID %d", list->active->phantom->id);
    return true;
}

// Move to the previous phantom in the list and make it active
bool phantom_list_prev(PhantomList_t *list) {
    if (!list) {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "NULL list in phantom_list_prev");
        return false;
    }
    
    if (!list->active || !list->active->prev) {
        return false;
    }
    
    list->active = list->active->prev;
    LOG_INFO("Moved to previous phantom, ID %d", list->active->phantom->id);
    return true;
}
