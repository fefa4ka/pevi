# Phantom Manager Implementation

## Overview

The Phantom Manager is a core component of Pevi that handles the creation, management, and rendering of multiple phantom objects (text panes) in 3D space. It's designed to support a dynamic number of phantoms that can be created and destroyed at runtime.

## Design Principles

1. **Dynamic Allocation**: Phantoms are allocated on demand, with the array of phantom pointers growing as needed.
2. **Efficient Management**: The manager provides functions for adding, removing, and accessing phantoms.
3. **Active Phantom Tracking**: One phantom is designated as "active" for input handling.
4. **Memory Safety**: All memory is properly tracked and freed when no longer needed.

## Implementation Details

### Data Structure

The Phantom Manager uses a dynamically resizable array of phantom pointers:

```c
typedef struct {
    Phantom_t **phantoms;       // Dynamic array of phantom pointers
    int count;                  // Number of phantoms currently in the array
    int capacity;               // Current capacity of the phantoms array
    int active_index;           // Index of the currently active phantom (-1 if none)
    int next_id;                // Next ID to assign to a new phantom
} PhantomManager_t;
```

### Memory Management

- **Initial Allocation**: The manager starts with a small capacity (e.g., 4 phantoms).
- **Growth Strategy**: When the array is full, its capacity is doubled.
- **Shrinking**: The array could be shrunk when it becomes significantly underutilized (not currently implemented).

### Key Operations

#### Creation and Initialization

```c
PhantomManager_t *phantom_manager_create(void);
```
- Allocates memory for the manager structure
- Initializes the phantoms array with initial capacity
- Sets count to 0, active_index to -1, and next_id to 1

#### Adding Phantoms

```c
int phantom_manager_add(PhantomManager_t *manager, Phantom_t *phantom);
int phantom_manager_create_phantom(PhantomManager_t *manager, const char *filename);
```
- Adds an existing phantom or creates a new one
- Resizes the array if needed
- Assigns a unique ID to the phantom
- Returns the index of the added phantom

#### Removing Phantoms

```c
bool phantom_manager_remove(PhantomManager_t *manager, int index);
```
- Frees the phantom at the specified index
- Shifts all phantoms after it down by one
- Updates the active_index if needed

#### Accessing Phantoms

```c
Phantom_t *phantom_manager_get(PhantomManager_t *manager, int index);
Phantom_t *phantom_manager_get_by_id(PhantomManager_t *manager, int id);
Phantom_t *phantom_manager_get_active(PhantomManager_t *manager);
```
- Retrieves phantoms by index, ID, or active status

#### Managing the Active Phantom

```c
bool phantom_manager_set_active(PhantomManager_t *manager, int index);
```
- Sets the active phantom for input handling

#### Cleanup

```c
void phantom_manager_free(PhantomManager_t *manager);
```
- Frees all phantoms and the manager itself

### Rendering

```c
void phantom_manager_draw_all(PhantomManager_t *manager, Camera_t *camera, InputEvent_t *event);
```
- Draws all phantoms in the manager
- Handles input events for each phantom

## Usage Example

```c
// Create the phantom manager
PhantomManager_t *manager = phantom_manager_create();

// Create a phantom for a file
int index = phantom_manager_create_phantom(manager, "example.txt");

// Get the created phantom and customize it
Phantom_t *phantom = phantom_manager_get(manager, index);
phantom->font.font = my_font;
phantom->plane = my_plane;

// Draw all phantoms
InputEvent_t event = {0};
phantom_manager_draw_all(manager, &camera, &event);

// Clean up
phantom_manager_free(manager);
```

## Future Improvements

1. **Spatial Organization**: Implement automatic arrangement of phantoms in 3D space.
2. **Phantom Groups**: Allow grouping related phantoms together.
3. **Phantom Relationships**: Track relationships between phantoms (e.g., parent-child).
4. **Serialization**: Save and load phantom configurations.
5. **Phantom Templates**: Create phantoms from predefined templates.
