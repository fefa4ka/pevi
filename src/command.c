#include "command.h"
#include "error.h"
#include <raylib.h>
#include <stdio.h>
#include <string.h>

extern Command_t commands[];

// Initializes the command buffer.
static void command_buffer_init(CommandBuffer_t *cb) {
  cb->length = 0;
  cb->buffer[0] = '\0';
}

// Appends a character to the command buffer.
static void command_buffer_append(CommandBuffer_t *cb, char c) {
  if (cb->length < COMMAND_BUFFER_SIZE - 1) {
    cb->buffer[cb->length++] = c;
    cb->buffer[cb->length] = '\0';
  }
}

// Removes the last character (for handling backspace).

static void command_buffer_backspace(CommandBuffer_t *cb) {
  if (cb->length > 0) {
    cb->length--;
    cb->buffer[cb->length] = '\0';
  }
}

// Executes the command in the buffer if it matches a known command.
static void command_buffer_execute(CommandBuffer_t *cb) {
  if (!cb) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null command buffer parameter");
    return;
  }

  bool command_found = false;
  for (int i = 0; commands[i].name != NULL; i++) {
    if (strcmp(commands[i].name, cb->buffer) == 0) {
      // Execute the command function, passing the argument if available.
      if (commands[i].command_function) {
        commands[i].command_function(commands[i].arg);
        command_found = true;
        break;
      } else {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Command function is NULL");
      }
    }
  }
  
  if (!command_found) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_WARNING, "Unknown command");
    printf("Unknown command: %s\n", cb->buffer);
  }
  
  // Clear the buffer for the next command.
  command_buffer_init(cb);
}

bool command_buffer_update(CommandBuffer_t *cb) {
  if (!cb) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null command buffer parameter");
    return false;
  }

  // When ENTER is pressed, execute the command.
  if (IsKeyPressed(KEY_ENTER)) {
    command_buffer_execute(cb);
    return true;
  }
  // Handle backspace to remove last character.
  else if (IsKeyPressed(KEY_BACKSPACE)) {
    command_buffer_backspace(cb);
  }
  // Otherwise, assume key is a printable character.
  else {
    int key = GetCharPressed();
    if (key) {
      command_buffer_append(cb, (char)key);
    }
  }
  return false;
}
