#pragma once
#include "core.h"
#include "font.h"
#include <camera.h>

typedef struct {
  Font *font;
  float font_size;
  int index;
  Vector3 size;
} Glyph_t;

Glyph_t symbol_glyph(char symbol, Font *font, float font_size);
void symbol_draw(Glyph_t *symbol, Vector3 pos_orig, Color tint, bool backface);
void text_draw(FontSettings *settings, char *content, Plane *plane, Camera_t *camera);
void text_draw_on_plane(FontSettings *settings, char *content, Plane *plane, Camera_t *camera);
static Vector3 symbol_measure(Font font, char symbol, float font_size);
static Vector3 text_measure(Font font, const char *text, float fontSize,
                            float fontSpacing, float lineSpacing);
