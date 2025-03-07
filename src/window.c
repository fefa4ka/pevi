#include "window.h"
#include "core.h"
#include "font.h"
#include "error.h"
#include "logger.h"

bool window_open(Window_t *window) {
  if (!window) {
    LOG_ERROR("Null window parameter");
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null window parameter");
    return false;
  }

  LOG_INFO("Opening window: %s (%dx%d)", window->title, window->width, window->height);
  
  LOG_DEBUG("Setting window config flags");
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_TRANSPARENT |
                 FLAG_VSYNC_HINT);
  
  LOG_DEBUG("Initializing window");
  InitWindow(window->width, window->height, window->title);
  
  // Check if window was created successfully
  if (!IsWindowReady()) {
    LOG_ERROR("Failed to initialize window");
    ERROR_SET(ERROR_UNKNOWN, ERROR_FATAL, "Failed to initialize window");
    return false;
  }
  
  LOG_DEBUG("Toggling fullscreen");
  ToggleFullscreen();
  
  LOG_DEBUG("Toggling borderless windowed mode");
  ToggleBorderlessWindowed();

  LOG_DEBUG("Setting target FPS: %d", window->fps_target);
  SetTargetFPS(window->fps_target);
  
  LOG_DEBUG("Disabling default exit key");
  SetExitKey(0);
  
  LOG_INFO("Window opened successfully");
  return true;
}

void window_close(Window_t *window) {
  LOG_INFO("Closing window");
  
  if (!window) {
    LOG_WARNING("Null window parameter in window_close");
    return;
  }
  
  // Unload font and shader if they exist
  if (window->font.texture.id > 0) {
    LOG_DEBUG("Unloading window font");
    UnloadFont(window->font);
  }
  
  if (window->shader.id > 0) {
    LOG_DEBUG("Unloading window shader");
    UnloadShader(window->shader);
  }

  LOG_DEBUG("Closing window");
  CloseWindow();
  LOG_INFO("Window closed");
}
