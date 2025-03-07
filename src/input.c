#include "input.h"
#include "command.h"
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialize the input handler
void input_init(InputHandler_t *handler) {
  // Clear the current and previous event structures
  memset(&handler->current_event, 0, sizeof(InputEvent_t));
  memset(&handler->previous_event, 0, sizeof(InputEvent_t));
  
  // Set up mode-specific handlers
  handler->mode_handlers[PEVI_MODE_FREE] = input_handle_free_mode;
  handler->mode_handlers[PEVI_MODE_EDIT] = input_handle_edit_mode;
  handler->mode_handlers[PEVI_MODE_COMMAND] = input_handle_command_mode;
  handler->mode_handlers[PEVI_MODE_DRAG_START] = NULL; // Not implemented yet
  handler->mode_handlers[PEVI_MODE_DRAG_ON] = NULL;    // Not implemented yet
  
  handler->cached_key = 0;
}

// Main input processing function
bool input_process(Pevi_t *pevi, Camera_t *camera, InputHandler_t *handler) {
  if (!pevi || !camera || !handler) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null parameter in input_process");
    return false;
  }

  // Update camera based on input
  camera_handle(camera);
  
  // Store previous event state
  handler->previous_event = handler->current_event;
  
  // Reset current event
  handler->current_event.source_type = INPUT_SOURCE_NONE;
  handler->current_event.mouse = INPUT_MOUSE_NONE;
  handler->current_event.key_type = INPUT_KEY_NONE;
  
  // Update mouse state
  handler->current_event.mouse_position = GetMousePosition();
  handler->current_event.mouse_delta = GetMouseDelta();
  handler->current_event.mouse_wheel = GetMouseWheelMove();
  
  // Update keyboard modifier state
  handler->current_event.shift_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
  handler->current_event.ctrl_down = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
  handler->current_event.alt_down = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
  
  // Check for mode switching first
  if (input_check_mode_switch(pevi, &handler->current_event, camera)) {
    return true; // Mode switch occurred, don't process further
  }
  
  // Call the appropriate mode handler
  if (handler->mode_handlers[pevi->mode] != NULL) {
    handler->mode_handlers[pevi->mode](pevi, &handler->current_event);
    return true;
  } else if (pevi->mode >= 0 && pevi->mode < 5) {
    // Valid mode but no handler
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_WARNING, "No handler for current mode");
    return false;
  } else {
    // Invalid mode
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Invalid mode");
    return false;
  }
}

// Handle input in free navigation mode
void input_handle_free_mode(Pevi_t *pevi, InputEvent_t *event) {
  // In free mode, most camera movement is handled by camera_handle
  // Here we just handle additional inputs specific to this mode
  
  // Check for special key presses
  if (IsKeyPressed(KEY_SPACE)) {
    printf("Space key pressed in free mode!\n");
  }
}

// Handle input in edit mode
void input_handle_edit_mode(Pevi_t *pevi, InputEvent_t *event) {
  // Get character input
  int key = GetCharPressed();
  if (key) {
    event->source_type = INPUT_SOURCE_KEYBOARD;
    event->key_type = INPUT_KEY_CHAR;
    event->key_code = key;
    printf("Character pressed in edit mode: %c\n", key);
  }
  
  // Check for special keys
  if (IsKeyPressed(KEY_BACKSPACE)) {
    event->source_type = INPUT_SOURCE_KEYBOARD;
    event->key_type = INPUT_KEY_SPECIAL;
    event->key_code = KEY_BACKSPACE;
    printf("Backspace pressed in edit mode\n");
  }
  else if (IsKeyPressed(KEY_DELETE)) {
    event->source_type = INPUT_SOURCE_KEYBOARD;
    event->key_type = INPUT_KEY_SPECIAL;
    event->key_code = KEY_DELETE;
    printf("Delete pressed in edit mode\n");
  }
  else if (IsKeyPressed(KEY_ENTER)) {
    event->source_type = INPUT_SOURCE_KEYBOARD;
    event->key_type = INPUT_KEY_SPECIAL;
    event->key_code = KEY_ENTER;
    printf("Enter pressed in edit mode\n");
  }
}

// Handle input in command mode
void input_handle_command_mode(Pevi_t *pevi, InputEvent_t *event) {
  // In command mode, we update the command buffer
  if (command_buffer_update(&pevi->command_buffer)) {
    // Command was executed, return to free mode
    pevi->mode = PEVI_MODE_FREE;
    event->source_type = INPUT_SOURCE_COMMAND;
  }
}

// Check for mode switching
bool input_check_mode_switch(Pevi_t *pevi, InputEvent_t *event, Camera_t *camera) {
  // Check for mode switching based on current mode
  if (pevi->mode == PEVI_MODE_FREE) {
    int key = GetCharPressed();
    if (key == 'e') {
      printf("Switching to Edit mode\n");
      pevi->mode = PEVI_MODE_EDIT;
      camera_set_mode(camera, PEVI_MODE_EDIT);
      return true;
    } 
    else if (key == ':') {
      printf("Switching to Command mode\n");
      pevi->mode = PEVI_MODE_COMMAND;
      camera_set_mode(camera, PEVI_MODE_COMMAND);
      return true;
    }
  } 
  else if (pevi->mode != PEVI_MODE_FREE) {
    // From any non-free mode, ESC returns to free mode
    if (IsKeyPressed(KEY_ESCAPE)) {
      printf("Returning to Free mode\n");
      pevi->mode = PEVI_MODE_FREE;
      camera_set_mode(camera, PEVI_MODE_FREE);
      return true;
    }
  }
  
  return false;
}

// Helper function to check if a key is pressed
bool input_is_key_pressed(int key) {
  return IsKeyPressed(key);
}

// Helper function to get a character pressed
int input_get_char_pressed(void) {
  return GetCharPressed();
}
