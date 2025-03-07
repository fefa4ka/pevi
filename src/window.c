#include "window.h"
#include "core.h"
#include "font.h"

void window_open(Window_t *window) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_TRANSPARENT |
                 FLAG_VSYNC_HINT);
  InitWindow(window->width, window->height, window->title);
  ToggleFullscreen();
  ToggleBorderlessWindowed();

  SetTargetFPS(window->fps_target);

  SetExitKey(0);
}

void window_close(Window_t *window) {
  UnloadFont(window->font);
  UnloadShader(window->shader);

  CloseWindow();
}
