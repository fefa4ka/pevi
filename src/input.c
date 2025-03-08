#include "input.h"
#include "command.h"
#include "error.h"
#include "logger.h"
#include "phantom.h"
#include "phantom_list.h"
#include "lr_file.h"
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
    LOG_DEBUG("Space key pressed in free mode");
  }
}

// Forward declaration of handle_edit_input
static void handle_edit_input(Phantom_t *phantom, InputEvent_t *event);

// Handle input in edit mode
void input_handle_edit_mode(Pevi_t *pevi, InputEvent_t *event) {
  // Get character input
  int key = GetCharPressed();
  if (key) {
    event->source_type = INPUT_SOURCE_KEYBOARD;
    event->key_type = INPUT_KEY_CHAR;
    event->key_code = key;
    LOG_DEBUG("Character pressed in edit mode: %c", key);
  }
  
  // Check for special keys
  if (IsKeyPressed(KEY_BACKSPACE)) {
    event->source_type = INPUT_SOURCE_KEYBOARD;
    event->key_type = INPUT_KEY_SPECIAL;
    event->key_code = KEY_BACKSPACE;
    LOG_DEBUG("Backspace pressed in edit mode");
  }
  else if (IsKeyPressed(KEY_DELETE)) {
    event->source_type = INPUT_SOURCE_KEYBOARD;
    event->key_type = INPUT_KEY_SPECIAL;
    event->key_code = KEY_DELETE;
    LOG_DEBUG("Delete pressed in edit mode");
  }
  else if (IsKeyPressed(KEY_ENTER)) {
    event->source_type = INPUT_SOURCE_KEYBOARD;
    event->key_type = INPUT_KEY_SPECIAL;
    event->key_code = KEY_ENTER;
    LOG_DEBUG("Enter pressed in edit mode");
  }
  
  // Process the edit input for the active phantom
  Phantom_t *active_phantom = phantom_list_get_active(pevi->phantoms);
  if (active_phantom) {
    handle_edit_input(active_phantom, event);
  } else {
    LOG_WARNING("No active phantom to handle edit input");
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
      // Only switch to edit mode if there's an active phantom
      Phantom_t *active = phantom_list_get_active(pevi->phantoms);
      if (active) {
        LOG_INFO("Switching to Edit mode");
        pevi->mode = PEVI_MODE_EDIT;
        camera_set_mode(camera, PEVI_MODE_EDIT);
        return true;
      } else {
        LOG_WARNING("Cannot switch to Edit mode: no active phantom");
      }
    } 
    else if (key == ':') {
      LOG_INFO("Switching to Command mode");
      pevi->mode = PEVI_MODE_COMMAND;
      camera_set_mode(camera, PEVI_MODE_COMMAND);
      return true;
    }

  } 
  else if (pevi->mode != PEVI_MODE_FREE) {
    // From any non-free mode, ESC returns to free mode
    if (IsKeyPressed(KEY_ESCAPE)) {
      LOG_INFO("Returning to Free mode");
      camera_set_mode(camera, PEVI_MODE_FREE);
      pevi->mode = PEVI_MODE_FREE;
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

// Handle edit mode input for a phantom
static void handle_edit_input(Phantom_t *phantom, InputEvent_t *event) {
  if (!event) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null event parameter");
    return;
  }
  
  if (!phantom || !phantom->buffer) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Invalid phantom or phantom has no buffer");
    return;
  }

  if (event->source_type == INPUT_SOURCE_KEYBOARD) {
    if (event->key_type == INPUT_KEY_CHAR) {
      int key = event->key_code;
      lr_result_t result;
      
      if (phantom->cursor.pos == 1) {
        result = lr_insert(&phantom->buffer->lr, key, phantom->cursor.line_no, phantom->cursor.pos);
        if (result != LR_SUCCESS) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to insert character at beginning of line");
          return;
        }
        phantom->cursor.needle = phantom->cursor.needle->next;
        phantom->cursor.pos++;
      } else if (phantom->cursor.is_eof) {
        result = lr_put(&phantom->buffer->lr, key, phantom->cursor.line_no);
        if (result != LR_SUCCESS) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to append character at end of line");
          return;
        }
        phantom->cursor.needle = lr_owner_tail(phantom->cursor.owner);
        phantom->cursor.pos++;
      } else {
        result = lr_insert_next(&phantom->buffer->lr, key, phantom->cursor.needle);
        if (result != LR_SUCCESS) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to insert character in middle of line");
          return;
        }
        phantom->cursor.pos++;
        phantom->cursor.needle = phantom->cursor.needle->next;
      }
    } else if (event->key_type == INPUT_KEY_SPECIAL) {
      if (event->key_code == KEY_BACKSPACE) {
        lr_result_t result;
        
        if (phantom->cursor.is_eof) {
          result = lr_pop(&phantom->buffer->lr, &phantom->cursor.needle->data, phantom->cursor.line_no);
          if (result != LR_SUCCESS) {
            ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to delete character at end of line");
            return;
          }
          phantom->cursor.pos--;
          phantom->cursor.needle = lr_owner_tail(phantom->cursor.owner);
        } else {
          result = lr_pull(&phantom->buffer->lr, &phantom->cursor.needle->data, phantom->cursor.line_no, phantom->cursor.pos);
          if (result != LR_SUCCESS) {
            ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to delete character in middle of line");
            return;
          }
          phantom->cursor.pos--;
          phantom->cursor.needle = NULL;
        }
        LOG_DEBUG("Cursor: %lu.%lu", phantom->cursor.line_no, phantom->cursor.pos);
      } else if (event->key_code == KEY_ENTER) {
        // Split the current line at cursor position
        lr_result_t result = lr_file_split(&phantom->buffer->lr, phantom->cursor.line_no, phantom->cursor.pos);
        if (result != LR_SUCCESS) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to split line");
          return;
        }

        // Move cursor to the beginning of the new line
        phantom->cursor.line_no++;
        phantom->cursor.pos = 1;

        // Update cursor needle to point to the beginning of the new line
        phantom->cursor.owner = lr_owner_find(&phantom->buffer->lr, lr_owner(phantom->cursor.line_no));
        if (!phantom->cursor.owner) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to find new line owner");
          return;
        }
        
        phantom->cursor.needle = lr_owner_head(&phantom->buffer->lr, phantom->cursor.owner);
        if (!phantom->cursor.needle) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to find new line head");
          return;
        }
        
        phantom->cursor.is_eof = false;
        phantom->line_to += 1;

        LOG_DEBUG("Line split. Cursor: %lu.%lu", phantom->cursor.line_no, phantom->cursor.pos);
      }
    }
  }
}
