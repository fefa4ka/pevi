#include "buffer.h"
#include "camera.h"
#include "command.h"
#include "config.h"
#include "core.h"
#include "error.h"
#include "font.h"
#include "input.h"
#include "logger.h"
#include "lr.h"
#include "lr_file.h"
#include "memory.h"
#include "raylib.h"
#include "render.h"
#include "text.h"
#include "window.h"
#include <stdio.h>
#include <stdlib.h>

Window_t window = {WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, FPS_TARGET};

Pevi_t pevi = {PEVI_MODE_FREE, true};

Camera_t camera = {
    {CAMERA_POSITION, CAMERA_TARGET, CAMERA_UP, CAMERA_FOVY, CAMERA_PROJECTION},
    CAMERA_MODE,
    true,
    false,
    true};

InputHandler_t input_handler;

Font_t font_default;

Plane plane;
Phantom_t phantom = {
    .font = {.font_size = 16, .spacing = 0.6, .line_spacing = 0.6},
    .line_from = 1,
    .line_to = 3};

void quit(void *arg) { pevi.is_closed = true; }
Command_t commands[] = {
    {"q", quit, &pevi.command_buffer}, {0} // End marker
};

void phantom_test(Pevi_t *pevi) {
  InputEvent_t event = {0};
  if (!phantom_draw_on_plane(&phantom, &camera, &event)) {
    ERROR_SET(ERROR_UNKNOWN, ERROR_WARNING, "Failed to draw phantom");
  }
  DrawGrid(100, 1.0f);
}

void handle_edit_input(InputEvent_t *event) {
  if (!event) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null event parameter");
    return;
  }
  
  if (!phantom.buffer) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Phantom has no buffer");
    return;
  }

  if (event->source_type == INPUT_SOURCE_KEYBOARD) {
    if (event->key_type == INPUT_KEY_CHAR) {
      int key = event->key_code;
      lr_result_t result;
      
      if (phantom.cursor.pos == 1) {
        result = lr_insert(&phantom.buffer->lr, key, phantom.cursor.line_no, phantom.cursor.pos);
        if (result != LR_SUCCESS) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to insert character at beginning of line");
          return;
        }
        phantom.cursor.needle = phantom.cursor.needle->next;
        phantom.cursor.pos++;
      } else if (phantom.cursor.is_eof) {
        result = lr_put(&phantom.buffer->lr, key, phantom.cursor.line_no);
        if (result != LR_SUCCESS) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to append character at end of line");
          return;
        }
        phantom.cursor.needle = lr_owner_tail(phantom.cursor.owner);
        phantom.cursor.pos++;
      } else {
        result = lr_insert_next(&phantom.buffer->lr, key, phantom.cursor.needle);
        if (result != LR_SUCCESS) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to insert character in middle of line");
          return;
        }
        phantom.cursor.pos++;
        phantom.cursor.needle = phantom.cursor.needle->next;
      }
    } else if (event->key_type == INPUT_KEY_SPECIAL) {
      if (event->key_code == KEY_BACKSPACE) {
        lr_result_t result;
        
        if (phantom.cursor.is_eof) {
          result = lr_pop(&phantom.buffer->lr, &phantom.cursor.needle->data, phantom.cursor.line_no);
          if (result != LR_SUCCESS) {
            ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to delete character at end of line");
            return;
          }
          phantom.cursor.pos--;
          phantom.cursor.needle = lr_owner_tail(phantom.cursor.owner);
        } else {
          result = lr_pull(&phantom.buffer->lr, &phantom.cursor.needle->data, phantom.cursor.line_no, phantom.cursor.pos);
          if (result != LR_SUCCESS) {
            ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to delete character in middle of line");
            return;
          }
          phantom.cursor.pos--;
          phantom.cursor.needle = NULL;
        }
        LOG_DEBUG("Cursor: %lu.%lu", phantom.cursor.line_no, phantom.cursor.pos);
      } else if (event->key_code == KEY_ENTER) {
        // Split the current line at cursor position
        lr_result_t result = lr_file_split(&phantom.buffer->lr, phantom.cursor.line_no, phantom.cursor.pos);
        if (result != LR_SUCCESS) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to split line");
          return;
        }

        // Move cursor to the beginning of the new line
        phantom.cursor.line_no++;
        phantom.cursor.pos = 1;

        // Update cursor needle to point to the beginning of the new line
        phantom.cursor.owner = lr_owner_find(&phantom.buffer->lr, lr_owner(phantom.cursor.line_no));
        if (!phantom.cursor.owner) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to find new line owner");
          return;
        }
        
        phantom.cursor.needle = lr_owner_head(&phantom.buffer->lr, phantom.cursor.owner);
        if (!phantom.cursor.needle) {
          ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_WARNING, "Failed to find new line head");
          return;
        }
        
        phantom.cursor.is_eof = false;
        phantom.line_to += 1;

        LOG_DEBUG("Line split. Cursor: %lu.%lu", phantom.cursor.line_no, phantom.cursor.pos);
      }
    }
  }
}

void render(Camera_t *camera, render_body_fn Render3DBody,
            render_body_fn Render2DBody) {
  if (!camera || !Render3DBody || !Render2DBody) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_FATAL, "Null parameter in render function");
    return;
  }
  
  while (!pevi.is_closed) {
    // Process input
    input_process(&pevi, camera, &input_handler);

    // Handle edit mode input
    if (pevi.mode == PEVI_MODE_EDIT) {
      handle_edit_input(&input_handler.current_event);
    }

    // Render the frame
    render_frame(&pevi, camera->camera, Render3DBody, Render2DBody);
    
    // Check for errors that occurred during rendering
    if (ERROR_CHECK()) {
      ErrorContext_t error = ERROR_GET();
      if (error.level == ERROR_FATAL) {
        pevi.is_closed = true;
      } else {
        // Clear non-fatal errors to continue rendering
        ERROR_CLEAR();
      }
    }
  }
}

bool resource_load() {
  bool success = true;
  
  // Try to load font, fallback to default if not found
  font_default = font_load("assets/fonts/FiraCode-Regular.ttf", "assets/shaders/sdf.fs");

  // If font loading failed, use a default font
  if (font_default.face.texture.id == 0) {
    ERROR_SET(ERROR_FONT_LOAD, ERROR_WARNING, "Failed to load custom font, using default font");
    font_default.face = GetFontDefault();
    font_default.shader = LoadShader(0, 0); // Use default shader
    success = false;
  }

  plane = camera_plane(&camera.camera);
  phantom.font.font = font_default;
  phantom.plane = plane;

  // Create a test file if it doesn't exist
  FILE *test_file = fopen("test.txt", "r");
  if (test_file == NULL) {
    test_file = fopen("test.txt", "w");
    if (test_file != NULL) {
      if (fprintf(test_file, "Welcome to Pevi!\nThis is a test file.\nYou can edit "
                         "this text in 3D space.") < 0) {
        ERROR_SET(ERROR_FILE_ACCESS, ERROR_WARNING, "Failed to write to test file");
        success = false;
      }
      fclose(test_file);
    } else {
      ERROR_SET(ERROR_FILE_ACCESS, ERROR_WARNING, "Failed to create test file");
      success = false;
    }
  } else {
    fclose(test_file);
  }

  // Open the buffer
  phantom.buffer = buffer_open("test.txt");
  if (!phantom.buffer) {
    ERROR_SET(ERROR_BUFFER_OPERATION, ERROR_ERROR, "Failed to open buffer");
    success = false;
  }

  return success;
}

int main(void) {
  // Initialize logger
  logger_init();
  logger_set_console_level(PEVI_LOG_DEBUG);
  logger_set_log_to_file(true, "pevi.log");
  
  LOG_INFO("Starting Pevi editor");
  
  // Initialize memory tracking
  memory_init();
  LOG_DEBUG("Memory tracking initialized");
  
  // Initialize error handling
  error_init();
  LOG_DEBUG("Error handling initialized");
  
  // Open window
  LOG_INFO("Opening window");
  if (!window_open(&window)) {
    ERROR_SET(ERROR_UNKNOWN, ERROR_FATAL, "Failed to open window");
    return EXIT_FAILURE;
  }

  // Initialize core components
  LOG_INFO("Initializing core components");
  if (!core_init()) {
    ERROR_SET(ERROR_UNKNOWN, ERROR_FATAL, "Failed to initialize core components");
    window_close(&window);
    return EXIT_FAILURE;
  }

  // Initialize input handler
  LOG_INFO("Initializing input handler");
  input_init(&input_handler);

  // Load resources
  LOG_INFO("Loading resources");
  if (!resource_load()) {
    ERROR_SET(ERROR_UNKNOWN, ERROR_ERROR, "Failed to load resources");
    LOG_WARNING("Using default resources due to loading failure");
    // Continue with default resources
  }

  // Main render loop
  LOG_INFO("Entering main render loop");
  render(&camera, phantom_test, render_ui);

  // Cleanup resources
  LOG_INFO("Cleaning up resources");
  if (phantom.buffer) {
    buffer_free(phantom.buffer);
    phantom.buffer = NULL;
  }
  
  // Cleanup window
  LOG_INFO("Closing window");
  window_close(&window);
  
  // Check for memory leaks
  LOG_INFO("Checking for memory leaks");
  memory_print_leaks();
  
  // Final memory cleanup
  LOG_INFO("Performing final memory cleanup");
  memory_cleanup();
  
  // Cleanup logger
  LOG_INFO("Shutting down logger");
  logger_cleanup();
  
  return EXIT_SUCCESS;
}
