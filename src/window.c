#include "window.h"
#include "core.h"
#include "font.h"
#include "error.h"

bool window_open(Window_t *window) {
  if (!window) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null window parameter");
    return false;
  }

  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_TRANSPARENT |
                 FLAG_VSYNC_HINT);
  InitWindow(window->width, window->height, window->title);
  
  // Check if window was created successfully
  if (!IsWindowReady()) {
    ERROR_SET(ERROR_UNKNOWN, ERROR_FATAL, "Failed to initialize window");
    return false;
  }
  
  ToggleFullscreen();
  ToggleBorderlessWindowed();

  SetTargetFPS(window->fps_target);
  SetExitKey(0);
  
  return true;
}

void window_close(Window_t *window) {
  UnloadFont(window->font);
  UnloadShader(window->shader);

  CloseWindow();
}
