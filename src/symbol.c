#include "symbol.h"
#include "raylib.h"
#include "error.h"
#include <raymath.h>
#include <rlgl.h>
#include <stdio.h>

Glyph_t symbol_glyph(char symbol, Font *font, float font_size) {
  Font glyph_font = *font;

  int codepointByteCount = 0;
  int codepoint = GetCodepoint(&symbol, &codepointByteCount);
  int index = GetGlyphIndex(glyph_font, codepoint);
  Vector3 size = symbol_measure(glyph_font, symbol, font_size);

  return (Glyph_t){font, font_size, index, size};
}

// Computes the scale factor for the symbol.
static float symbol_scale_get(Glyph_t *symbol) {
    Font *font = symbol->font;
    return symbol->font_size / (float)font->baseSize;
}

// Computes the drawing position for the symbol based on its glyph offset and font padding.
static Vector3 symbol_position_get(Glyph_t *symbol, Vector3 pos_orig, float scale) {
    Font *font = symbol->font;
    GlyphInfo glyph = font->glyphs[symbol->index];
    Vector3 pos;
    pos.x = pos_orig.x + ((float)(glyph.offsetX - font->glyphPadding) / font->baseSize) * scale;
    pos.y = pos_orig.y;
    pos.z = pos_orig.z + ((float)(glyph.offsetY - font->glyphPadding) / font->baseSize) * scale;
    return pos;
}

// Computes the source rectangle of the glyph including padding.
static Rectangle symbol_src_rec_get(Glyph_t *symbol) {
    Font *font = symbol->font;
    Rectangle glyph_rec = font->recs[symbol->index];
    float glyph_padding = (float)font->glyphPadding;
    Rectangle src_rec;
    src_rec.x = glyph_rec.x - glyph_padding;
    src_rec.y = glyph_rec.y - glyph_padding;
    src_rec.width = glyph_rec.width + 2.0f * glyph_padding;
    src_rec.height = glyph_rec.height + 2.0f * glyph_padding;
    return src_rec;
}

// Computes the drawing dimensions (width and height) for the symbol.
static void symbol_dimensions_get(const Font *font, const Rectangle *src_rec, float scale, float *width, float *height) {
    *width  = (src_rec->width / font->baseSize) * scale;
    *height = (src_rec->height / font->baseSize) * scale;
}

// Computes the texture coordinates for the symbol.
static void symbol_texture_coords_get(const Font *font, const Rectangle *src_rec, 
                                        float *tx, float *ty, float *tw, float *th) {
    *tx = src_rec->x / font->texture.width;
    *ty = src_rec->y / font->texture.height;
    *tw = (src_rec->x + src_rec->width) / font->texture.width;
    *th = (src_rec->y + src_rec->height) / font->texture.height;
}

// Draws the symbol's quads (front face and, optionally, back face).
static void symbol_draw_quads(float width, float height, Color tint, bool backface,
                                float tx, float ty, float tw, float th) {
    const float x = 0.0f;
    const float y = 0.0f;
    const float z = 0.0f;

    rlBegin(RL_QUADS);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);

    // Front Face
    rlNormal3f(0.0f, 1.0f, 0.0f);
    rlTexCoord2f(tx, ty);
    rlVertex3f(x, y, z);
    rlTexCoord2f(tx, th);
    rlVertex3f(x, y, z + height);
    rlTexCoord2f(tw, th);
    rlVertex3f(x + width, y, z + height);
    rlTexCoord2f(tw, ty);
    rlVertex3f(x + width, y, z);

    // Back Face (if enabled)
    if (backface) {
        rlNormal3f(0.0f, -1.0f, 0.0f);
        rlTexCoord2f(tx, ty);
        rlVertex3f(x, y, z);
        rlTexCoord2f(tw, ty);
        rlVertex3f(x + width, y, z);
        rlTexCoord2f(tw, th);
        rlVertex3f(x + width, y, z + height);
        rlTexCoord2f(tx, th);
        rlVertex3f(x, y, z + height);
    }
    rlEnd();
}

// Draws the symbol (glyph) at the given origin with the specified tint and backface option.
void symbol_draw(Glyph_t *symbol, Vector3 pos_orig, Color tint, bool backface) {
    Font *font = symbol->font;
    float scale = symbol_scale_get(symbol);

    Vector3 pos = symbol_position_get(symbol, pos_orig, scale);
    Rectangle src_rec = symbol_src_rec_get(symbol);

    float width, height;
    symbol_dimensions_get(font, &src_rec, scale, &width, &height);

    float tx, ty, tw, th;
    symbol_texture_coords_get(font, &src_rec, &tx, &ty, &tw, &th);

    rlCheckRenderBatchLimit(4 + 4 * backface);
    rlSetTexture(font->texture.id);

    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);

    symbol_draw_quads(width, height, tint, backface, tx, ty, tw, th);

    rlPopMatrix();
    rlSetTexture(0);
}

void symbol_interaction_handle(char symbol, Vector3 pos, Vector3 glyph_size,
                               InputEvent_t *event) {
  Vector3 cursor_size = glyph_size;
  cursor_size.y = 0;

  event->source_type = INPUT_SOURCE_SYMBOL;

  rlDisableDepthTest();
  DrawCubeWiresV(
      (Vector3){pos.x + glyph_size.x / 2, 0, pos.z + glyph_size.z / 2},
      cursor_size, RED);

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    event->mouse = INPUT_MOUSE_CLICK;
    LOG_DEBUG("CLICK %c", symbol);
  } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    event->mouse = INPUT_MOUSE_DRAG;
    LOG_DEBUG("DRAG %c", symbol);
  } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    event->mouse = INPUT_MOUSE_RELEASE;
    LOG_DEBUG("RELEASE %c", symbol);
  }
}

static Vector3 symbol_measure(Font font, char symbol, float font_size) {
  const float font_base_size = (float)font.baseSize;
  const float scale = font_size / font_base_size;
  float text_width = 0.0f;
  const float text_height = scale; // A single symbol has one line height

  // Create a pointer to the symbol to decode the codepoint.
  // (Note: The symbol is treated as a one-character string.)
  const char *symbol_ptr = &symbol;
  int codepoint_byte_count = 0;
  int codepoint = GetCodepoint(&symbol, &codepoint_byte_count);

  // The glyph index for the given codepoint.
  int glyph_index = GetGlyphIndex(font, codepoint);

  // Compute the scaled glyph advance using our helper.
  text_width +=
      font_glyph_advance_scaled_get(&font, codepoint, scale, font_base_size);

  Vector3 result = {0};
  result.x = text_width;
  result.y = 0;
  result.z = text_height;
  return result;
}
