#include "input.h"
#include "command.h"
#include "error.h"
#include "logger.h"
#include "lr_text.h"
#include "phantom.h"
#include "phantom_list.h"
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool input_check_empty_space_click(Pevi_t *pevi, InputEvent_t *event);

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
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR,
              "Null parameter in input_process");
    return false;
  }

  if (IsKeyDown(KEY_LEFT_SHIFT)) {
    camera->is_mouse_enabled = true;
  } else {
    camera->is_mouse_enabled = false;
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
  handler->current_event.shift_down =
      IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
  handler->current_event.ctrl_down =
      IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
  handler->current_event.alt_down =
      IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);

  // Check for clicks in empty space first
  if (input_check_empty_space_click(pevi, &handler->current_event)) {
    return true; // Empty space click handled
  }

  // Check for mode switching next
  if (input_check_mode_switch(pevi, &handler->current_event, camera)) {
    return true; // Mode switch occurred, don't process further
  }

  // Call the appropriate mode handler
  if (handler->mode_handlers[pevi->mode] != NULL) {
    handler->mode_handlers[pevi->mode](pevi, &handler->current_event);
    return true;
  } else if (pevi->mode >= 0 && pevi->mode < 5) {
    // Valid mode but no handler
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_WARNING,
              "No handler for current mode");
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
  } else if (IsKeyPressed(KEY_DELETE)) {
    event->source_type = INPUT_SOURCE_KEYBOARD;
    event->key_type = INPUT_KEY_SPECIAL;
    event->key_code = KEY_DELETE;
    LOG_DEBUG("Delete pressed in edit mode");
  } else if (IsKeyPressed(KEY_ENTER)) {
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

// Check if mouse clicked in empty space
static bool input_check_empty_space_click(Pevi_t *pevi, InputEvent_t *event) {
  // Only process in FREE mode when left mouse button is clicked
  if (pevi->mode != PEVI_MODE_FREE ||
      !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    return false;
  }

  // If the event has no source type, it means the click was in empty space
  if (event->source_type == INPUT_SOURCE_NONE) {
    LOG_DEBUG("Click detected in empty space");

    // Unselect the active phantom if there is one
    if (pevi->phantoms) {
      Phantom_t *active = phantom_list_get_active(pevi->phantoms);
      if (active) {
        active->is_selected = false;
        LOG_DEBUG("Unselected active phantom with ID %d", active->id);
      }
    }

    return true;
  }

  return false;
}

// Check for mode switching
bool input_check_mode_switch(Pevi_t *pevi, InputEvent_t *event,
                             Camera_t *camera) {
  // Check for mode switching based on current mode
  if (pevi->mode == PEVI_MODE_FREE) {
    int key = GetCharPressed();
    if (key == 'i' || key == 'I') {
      // Only switch to edit mode if there's an active phantom
      Phantom_t *active = phantom_list_get_active(pevi->phantoms);
      if (active) {
        LOG_INFO("Switching to Edit mode");
        pevi->mode = PEVI_MODE_EDIT;
        if (key == 'I') {
          LOG_DEBUG("Cursor position: %lu.%lu", active->cursor.line_no,
                    active->cursor.pos);
          if (active->cursor.pos > 0) {
            active->cursor.pos -= 1;
          }
          LOG_DEBUG("Cursor position: %lu.%lu", active->cursor.line_no,
                    active->cursor.pos);
        }
        camera_set_mode(camera, PEVI_MODE_EDIT);
        return true;
      } else {
        LOG_WARNING("Cannot switch to Edit mode: no active phantom");
      }
    } else if (key == ':') {
      LOG_INFO("Switching to Command mode");
      pevi->mode = PEVI_MODE_COMMAND;
      camera_set_mode(camera, PEVI_MODE_COMMAND);
      return true;
    }

  } else if (pevi->mode != PEVI_MODE_FREE) {
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
bool input_is_key_pressed(int key) { return IsKeyPressed(key); }

// Helper function to get a character pressed
int input_get_char_pressed(void) { return GetCharPressed(); }

// Handle edit mode input for a phantom
static void handle_edit_input(Phantom_t *phantom, InputEvent_t *event) {
  if (!event) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null event parameter");
    return;
  }

  if (!phantom || !phantom->buffer) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR,
              "Invalid phantom or phantom has no buffer");
    return;
  }

  if (event->source_type == INPUT_SOURCE_KEYBOARD) {
    if (event->key_type == INPUT_KEY_CHAR) {
      int key = event->key_code;
      lr_result_t result;

      // Convert Unicode codepoint to UTF-8 bytes
      char utf8_buffer[5] = {0};
      int bytes = 0;

      // Convert codepoint to UTF-8 bytes
      if (key <= 0x7F) {
        // ASCII character (1 byte)
        utf8_buffer[0] = (char)key;
        bytes = 1;
      } else if (key <= 0x7FF) {
        // 2-byte character
        utf8_buffer[0] = 0xC0 | (key >> 6);
        utf8_buffer[1] = 0x80 | (key & 0x3F);
        LOG_DEBUG("2-byte character U+%04X for utf8_buffer: %s (len=%d)", key,
                  utf8_buffer, bytes);
        bytes = 2;
      } else if (key <= 0xFFFF) {
        // 3-byte character
        utf8_buffer[0] = 0xE0 | (key >> 12);
        utf8_buffer[1] = 0x80 | ((key >> 6) & 0x3F);
        utf8_buffer[2] = 0x80 | (key & 0x3F);
        bytes = 3;
      } else if (key <= 0x10FFFF) {
        // 4-byte character
        utf8_buffer[0] = 0xF0 | (key >> 18);
        utf8_buffer[1] = 0x80 | ((key >> 12) & 0x3F);
        utf8_buffer[2] = 0x80 | ((key >> 6) & 0x3F);
        utf8_buffer[3] = 0x80 | (key & 0x3F);
        bytes = 4;
      }

      // Insert each byte of the UTF-8 character
      for (int i = 0; i < bytes; i++) {
        if (phantom->cursor.pos == 0) {
          result = lr_insert(&phantom->buffer->lr, utf8_buffer[i],
                             phantom->cursor.line_no, i);
        } else if (phantom->cursor.is_eof) {
          result = lr_put(&phantom->buffer->lr, utf8_buffer[i],
                          phantom->cursor.line_no);
        } else {
          struct lr_cell *needle = phantom->cursor.needle;
          for (int j = 0; j < i; j++) {
            needle = needle->next;
          }
          result = lr_insert_next(&phantom->buffer->lr, utf8_buffer[i], needle);
        }

        if (result != LR_SUCCESS) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING,
                    "Failed to insert character byte");
          return;
        }
      }
      phantom->cursor.pos++;
    } else if (event->key_type == INPUT_KEY_SPECIAL) {
      if (event->key_code == KEY_BACKSPACE) {
        lr_result_t result;

        if (phantom->cursor.pos >
            0) { // Only delete if not at beginning of line
          if (phantom->cursor.is_eof) {
            result = lr_pop(&phantom->buffer->lr, &phantom->cursor.needle->data,
                            phantom->cursor.line_no);
            if (result != LR_SUCCESS) {
              ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING,
                        "Failed to delete character at end of line");
              return;
            }
            phantom->cursor.pos--;
            phantom->cursor.needle = lr_owner_tail(phantom->cursor.owner);
          } else {
            size_t i = phantom->cursor.char_pos;
            LOG_DEBUG("Char start: %p, char end next: %p",
                      phantom->cursor.char_start, phantom->cursor.char_end);
            struct lr_cell *needle = NULL;
            while (needle != phantom->cursor.char_end) {
              if (needle) {
                needle = needle->next;
              } else {
                needle = phantom->cursor.char_start;
              }

              lr_data_t data;
              result = lr_pull(&phantom->buffer->lr, &data,
                               phantom->cursor.line_no, i);
              LOG_DEBUG("Position: %lu.%lu(%zu), Needle: %p, needle->next: %p, "
                        "data: %d",
                        phantom->cursor.line_no, phantom->cursor.pos, i, needle,
                        needle->next, data);
              if (result != LR_SUCCESS) {
                ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING,
                          "Failed to delete character in middle of line");
                return;
              }
            }

            phantom->cursor.pos--;
            phantom->cursor.needle = NULL;
          }
        } else if (phantom->cursor.pos == 0) {
          if (phantom->cursor.line_no == 1) {
            LOG_DEBUG("Can't merge first line");
            // Can't merge first line
            return;
          }
          size_t line_prev_length =
              lr_count_owned(&phantom->buffer->lr, phantom->cursor.line_no - 1);
          result = lr_text_line_merge(&phantom->buffer->lr,
                                      phantom->cursor.line_no - 1,
                                      phantom->cursor.line_no);

          if (result != LR_SUCCESS) {
            ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING,
                      "Failed to merge lines");
            return;
          }

          phantom->cursor.line_no--;
          phantom->cursor.pos = line_prev_length;
        }
        LOG_DEBUG("Cursor: %lu.%lu", phantom->cursor.line_no,
                  phantom->cursor.pos);
      } else if (event->key_code == KEY_ENTER) {
        // Split the current line at cursor position
        lr_result_t result = lr_text_split(
            &phantom->buffer->lr, phantom->cursor.line_no, phantom->cursor.pos);
        if (result != LR_SUCCESS) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING,
                    "Failed to split line");
          return;
        }

        // Move cursor to the beginning of the new line
        phantom->cursor.line_no++;
        phantom->cursor.pos = 0;

        // Update cursor needle to point to the beginning of the new line
        phantom->cursor.owner = lr_owner_find(
            &phantom->buffer->lr, lr_owner(phantom->cursor.line_no));
        if (!phantom->cursor.owner) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING,
                    "Failed to find new line owner");
          return;
        }

        phantom->cursor.needle =
            lr_owner_head(&phantom->buffer->lr, phantom->cursor.owner);
        if (!phantom->cursor.needle) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING,
                    "Failed to find new line head");
          return;
        }

        phantom->cursor.is_eof = false;
        phantom->line_to += 1;

        LOG_DEBUG("Line split. Cursor: %lu.%lu", phantom->cursor.line_no,
                  phantom->cursor.pos);
      }
    }
  }
}
