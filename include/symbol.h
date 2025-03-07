#pragma once
#include "core.h"
#include "font.h"
#include "input.h"

typedef struct {
  Font *font;
  float font_size;
  int index;
  Vector3 size;
} Glyph_t;

Glyph_t symbol_glyph(char symbol, Font *font, float font_size);
void symbol_draw(Glyph_t *symbol, Vector3 pos_orig, Color tint, bool backface);
static Vector3 symbol_measure(Font font, char symbol, float font_size);
void symbol_interaction_handle(char symbol, Vector3 pos, Vector3 glyph_size, InputEvent_t *event);
