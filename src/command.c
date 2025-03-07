#include "command.h"
#include "error.h"
#include "logger.h"
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

  // Parse the command and arguments
  char cmd[COMMAND_BUFFER_SIZE] = {0};
  char arg[COMMAND_BUFFER_SIZE] = {0};
  
  // Extract command and argument (if any)
  char *space_pos = strchr(cb->buffer, ' ');
  if (space_pos) {
    // Command with argument
    int cmd_len = space_pos - cb->buffer;
    strncpy(cmd, cb->buffer, cmd_len);
    cmd[cmd_len] = '\0';
    
    // Skip any extra spaces
    while (*space_pos == ' ') space_pos++;
    
    // Copy the argument
    strcpy(arg, space_pos);
  } else {
    // Command without argument
    strcpy(cmd, cb->buffer);
  }
  
  bool command_found = false;
  for (int i = 0; commands[i].name != NULL; i++) {
    if (strcmp(commands[i].name, cmd) == 0) {
      // Execute the command function
      if (commands[i].command_function) {
        // If command has an argument in the buffer, use it
        // Otherwise use the default argument from the command table
        void *command_arg = *arg ? (void*)arg : commands[i].arg;
        commands[i].command_function(command_arg);
        command_found = true;
        break;
      } else {
        ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Command function is NULL");
      }
    }
  }
  
  if (!command_found) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_WARNING, "Unknown command");
    LOG_WARNING("Unknown command: %s", cmd);
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
