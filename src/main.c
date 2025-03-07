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


void render(Camera_t *camera, render_body_fn Render3DBody,
            render_body_fn Render2DBody) {
  if (!camera || !Render3DBody || !Render2DBody) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_FATAL, "Null parameter in render function");
    return;
  }
  
  while (!pevi.is_closed) {
    // Process input
    input_process(&pevi, camera, &input_handler);

    // Edit mode input is handled in input_handle_edit_mode

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
