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
#include "phantom_list.h"
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

void quit(void *arg) { pevi.is_closed = true; }

void next_phantom(void *arg) {
  if (pevi.phantoms) {
    phantom_list_next(pevi.phantoms);
  }
}

void prev_phantom(void *arg) {
  if (pevi.phantoms) {
    phantom_list_prev(pevi.phantoms);
  }
}

void new_phantom(void *arg) {
  if (pevi.phantoms) {
    PhantomNode_t *node = phantom_list_create_phantom(pevi.phantoms, "test.txt");
    if (node && node->phantom) {
      node->phantom->font.font = font_default;
      node->phantom->plane = plane;
      
      // Position the new phantom to the right of the active one
      Phantom_t *active = phantom_list_get_active(pevi.phantoms);
      if (active) {
        node->phantom->plane.pos.x = active->plane.pos.x + 5.0f;
      }
      
      // Make the new phantom active
      phantom_list_set_active(pevi.phantoms, node);
    }
  }
}

// Open a file as a new phantom
void open_file(void *arg) {
  if (!arg || !pevi.phantoms) return;
  
  const char *filename = (const char *)arg;
  LOG_INFO("Opening file: %s", filename);
  
  // Create a new phantom for the file
  PhantomNode_t *node = phantom_list_create_phantom(pevi.phantoms, filename);
  if (!node || !node->phantom) {
    LOG_ERROR("Failed to create phantom for file: %s", filename);
    return;
  }
  
  // Set up the phantom
  node->phantom->font.font = font_default;
  node->phantom->plane = camera_plane(&camera.camera);
  
  // Position the new phantom to the right of the active one
  Phantom_t *active = phantom_list_get_active(pevi.phantoms);
  if (active) {
    node->phantom->plane.pos.x = active->plane.pos.x + 5.0f;
  }
  
  // Make the new phantom active
  phantom_list_set_active(pevi.phantoms, node);
  LOG_INFO("Opened file as phantom: %s", filename);
  
  // Return to free mode
  pevi.mode = PEVI_MODE_FREE;
  camera_set_mode(&camera, PEVI_MODE_FREE);
}

Command_t commands[] = {
    {"q", quit, &pevi.command_buffer},
    {"n", next_phantom, NULL},
    {"p", prev_phantom, NULL},
    {"new", new_phantom, NULL},
    {"e", open_file, NULL},  // 'e' command for opening files
    {0} // End marker
};

void phantom_test(Pevi_t *pevi) {
  InputEvent_t event = {0};
  
  // Draw all phantoms using the phantom list
  if (pevi->phantoms) {
    phantom_list_draw_all(pevi->phantoms, &camera, &event);
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

  // Create a phantom list
  pevi.phantoms = phantom_list_create();
  if (!pevi.phantoms) {
    ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to create phantom list");
    return false;
  }

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

  // Create a phantom for the test file
  PhantomNode_t *node = phantom_list_create_phantom(pevi.phantoms, "test.txt");
  if (!node) {
    ERROR_SET(ERROR_UNKNOWN, ERROR_ERROR, "Failed to create phantom for test file");
    success = false;
    return success;
  }
  
  // Get the created phantom and set its properties
  Phantom_t *phantom = node->phantom;
  if (phantom) {
    phantom->font.font = font_default;
    phantom->plane = plane;
    LOG_INFO("Created phantom with ID %d", phantom->id);
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
  if (pevi.phantoms) {
    phantom_list_free(pevi.phantoms);
    pevi.phantoms = NULL;
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
