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
} FontSettings_t;

Font_t font_load(char *ttf_filename, char *shader_filename);

float font_glyph_advance_get(const Font *font, int codepoint);
float font_glyph_advance_scaled_get(const Font *font, int codepoint,
                                           float scale, float font_base_size);

