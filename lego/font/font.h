#pragma once
#include <core.h>

typedef struct {
  Font face;
  Shader shader;
} Font_t;

typedef struct {
  Font_t font;
  float font_size;
  float spacing;
  float line_spacing;
} FontSettings;

Font_t font_load(char *ttf_filename, char *shader_filename);
