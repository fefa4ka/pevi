#pragma once
#include <core.h>

#define RAYLIB_NEW_RLGL

typedef struct {
  char *title;
  unsigned int width;
  unsigned int height;
  unsigned int fps_target;

  Font font;
  Shader shader;
} Window_t;

bool window_open(Window_t *state);
void window_close(Window_t *state);
