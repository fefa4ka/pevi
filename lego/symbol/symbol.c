#include "symbol.h"
#include "raylib.h"
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

void symbol_draw(Glyph_t *symbol, Vector3 pos_orig, Color tint, bool backface) {

  Font *font = symbol->font;
  float scale = symbol->font_size / (float)font->baseSize;
  GlyphInfo glyph = symbol->font->glyphs[symbol->index];
  Rectangle glyph_rec = font->recs[symbol->index];
  float glyph_padding = (float)font->glyphPadding;

  Vector3 pos = {
      .x = pos_orig.x +
           (float)(glyph.offsetX - font->glyphPadding) / font->baseSize * scale,
      .y = pos_orig.y,
      .z = pos_orig.z +
           (float)(glyph.offsetY - font->glyphPadding) / font->baseSize * scale,
  };

  Rectangle srcRec = {.x = glyph_rec.x - glyph_padding,
                      .y = glyph_rec.y - glyph_padding,
                      .width = glyph_rec.width + 2.0f * glyph_padding,
                      .height = glyph_rec.height + 2.0f * glyph_padding};

  float width = srcRec.width / font->baseSize * scale;
  float height = srcRec.height / font->baseSize * scale;

  const float x = 0.0f;
  const float y = 0.0f;
  const float z = 0.0f;

  float tx = srcRec.x / font->texture.width;
  float ty = srcRec.y / font->texture.height;
  float tw = (srcRec.x + srcRec.width) / font->texture.width;
  float th = (srcRec.y + srcRec.height) / font->texture.height;

  rlCheckRenderBatchLimit(4 + 4 * backface);
  rlSetTexture(font->texture.id);

  rlPushMatrix();
  rlTranslatef(pos.x, pos.y, pos.z);

  rlBegin(RL_QUADS);
  rlColor4ub(tint.r, tint.g, tint.b, tint.a);

  // Front Face
  rlNormal3f(0.0f, 1.0f, 0.0f); // Normal Pointing Up
  rlTexCoord2f(tx, ty);
  rlVertex3f(x, y, z); // Top Left Of The Texture and Quad
  rlTexCoord2f(tx, th);
  rlVertex3f(x, y, z + height); // Bottom Left Of The Texture and Quad
  rlTexCoord2f(tw, th);
  rlVertex3f(x + width, y,
             z + height); // Bottom Right Of The Texture and Quad
  rlTexCoord2f(tw, ty);
  rlVertex3f(x + width, y, z); // Top Right Of The Texture and Quad

  if (backface) {
    // Back Face
    rlNormal3f(0.0f, -1.0f, 0.0f); // Normal Pointing Down
    rlTexCoord2f(tx, ty);
    rlVertex3f(x, y, z); // Top Right Of The Texture and Quad
    rlTexCoord2f(tw, ty);
    rlVertex3f(x + width, y, z); // Top Left Of The Texture and Quad
    rlTexCoord2f(tw, th);
    rlVertex3f(x + width, y,
               z + height); // Bottom Left Of The Texture and Quad
    rlTexCoord2f(tx, th);
    rlVertex3f(x, y,
               z + height); // Bottom Right Of The Texture and Quad
  }
  rlEnd();

  rlPopMatrix();

  rlSetTexture(0);
}

void text_draw_on_plane(FontSettings *settings, char *content, Plane *plane,
                        Camera_t *camera) {
  Vector3 pos = plane->pos;
  Vector3 angles = plane->angles;

  rlPushMatrix();
  rlTranslatef(pos.x, pos.y, pos.z);

  rlRotatef(RAD2DEG * angles.z, 0, 0, 1); // Roll (around Z-axis)
  rlRotatef(RAD2DEG * angles.y, 0, 1,
            0); // Rotate around Y-axis (yaw)
  rlRotatef(RAD2DEG * angles.x, 1, 0,
            0); // Rotate around X-axis (pitch)

  text_draw(settings, content, plane, camera);

  rlPopMatrix();
}

bool object_is_hovered(Camera_t *camera, BoundingBox object, Plane *plane) {
  Vector3 size = Vector3Subtract(object.max, object.min);
  Vector3 center = Vector3Add(object.min, Vector3Scale(size, 0.5f));
  // Build transformation matrix
  Matrix transform = MatrixIdentity();
  // First translate to origin
  transform =
      MatrixMultiply(transform, MatrixTranslate(center.x, -center.y, center.z));
  // Apply rotations
  transform = MatrixMultiply(transform, MatrixRotateX(plane->angles.x));
  transform = MatrixMultiply(transform, MatrixRotateY(plane->angles.y));
  transform = MatrixMultiply(transform, MatrixRotateZ(plane->angles.z));
  // Translate to final position
  transform = MatrixMultiply(
      transform, MatrixTranslate(plane->pos.x, plane->pos.y, plane->pos.z));

  // Create transformed bounding box
  Vector3 min_position = {-size.x / 2, -size.y / 2, -size.z / 2};
  Vector3 max_position = {size.x / 2, size.y / 2, size.z / 2};
  BoundingBox box = (BoundingBox){Vector3Transform(min_position, transform),
                                  Vector3Transform(max_position, transform)};

  // Check collisions
  return GetRayCollisionBox(camera->ray, box).hit ||
         GetRayCollisionBox(camera->ray_center, box).hit;
  return false;
}
void symbol_interaction_handle(char symbol, Vector3 pos, Vector3 glyph_size) {
  Vector3 cursor_size = glyph_size;
  cursor_size.y = 0;

  rlDisableDepthTest();
  DrawCubeWiresV(
      (Vector3){pos.x + glyph_size.x / 2, 0, pos.z + glyph_size.z / 2},
      cursor_size, RED);

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    printf("CLICK %c\n", symbol);
  } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    printf("DRAG %c\n", symbol);
  } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    printf("RELEASE %c\n", symbol);
  }
}

void text_draw(FontSettings *settings, char *content, Plane *plane,
               Camera_t *camera) {
  Font font = settings->font.face;
  float scale = settings->font_size / (float)font.baseSize;
  Vector3 size = text_measure(font, content, settings->font_size,
                              settings->spacing, settings->line_spacing);

  BoundingBox text_box = (BoundingBox){{0, 0, 0}, size};
  bool text_is_hovered = object_is_hovered(camera, text_box, plane);
  if (text_is_hovered) {
    DrawCubeV((Vector3){size.x / 2, -0.15, size.z / 2}, size, BLUE);
  }

  BeginShaderMode(settings->font.shader); // Activate SDF font shader
  Vector3 pos = {0, 0, 0};

  for (int index = 0; index < TextLength(content); index++) {
    Glyph_t glyph = symbol_glyph(content[index], &font, settings->font_size);

    if (content[index] == '\n') {
      pos.z += glyph.size.z;
      pos.x = 0;
      continue;
    }

    if (content[index] == '\t') {
      pos.x += settings->spacing * 2;
      continue;
    }

    symbol_draw(&glyph, pos, WHITE, false);

    if (text_is_hovered) {
      Vector3 glyph_size = glyph.size;
      glyph_size.y = 0;
      BoundingBox symbol_box = (BoundingBox){pos, Vector3Add(pos, glyph_size)};

      if (object_is_hovered(camera, symbol_box, plane)) {
        symbol_interaction_handle(content[index], pos, glyph.size);
      }
    }

    pos.x += glyph.size.x + settings->spacing / font.baseSize * scale;
  }

  EndShaderMode(); // Activate our default shader for next drawings
}

static Vector3 symbol_measure(Font font, char symbol, float fontSize) {
  float tempTextWidth = 0.0f; // Used to count longer text line width

  float scale = fontSize / (float)font.baseSize;
  float textHeight = scale;
  float textWidth = 0.0f;

  int letter = 0; // Current character
  int index = 0;  // Index position in sprite font

  int codepointByteCount = 0;
  letter = GetCodepoint(&symbol, &codepointByteCount);
  index = GetGlyphIndex(font, letter);

  if (font.glyphs[index].advanceX != 0)
    textWidth += (font.glyphs[index].advanceX) / (float)font.baseSize * scale;
  else
    textWidth += (font.recs[index].width + font.glyphs[index].offsetX) /
                 (float)font.baseSize * scale;

  Vector3 vec = {0};
  vec.x = textWidth; // Adds chars spacing to measure
  vec.y = 0.25f;
  vec.z = textHeight;

  return vec;
}

/*bool text_is_onscreen(FontSettings *settings, char *content, Camera3D
 * camera, TextPlane *plane){*/
/*  Font font = settings->font;*/
/*                Vector3RotateByAxisAngle*/
/*    Vector3 size  = text_measure(font, content,*/
/*                                 settings->font_size, settings->spacing,*/
/*                                 settings->line_spacing);*/
/*    Vector2 world_pos = GetWorldToScreen(pos, camera);*/
/**/
/*    Matrix transform;*/
/*    transform = MatrixTranslate(next_props->pos.x, next_props->pos.y,*/
/*                                next_props->pos.z);*/
/*    transform = MatrixMultiply(MatrixRotateY(next_props->angles.y),
 * transform);*/
/*    transform = MatrixMultiply(MatrixRotateX(next_props->angles.x),
 * transform);*/
/*    transform = MatrixMultiply(*/
/*        MatrixTranslate(state->size.x, state->size.y, state->size.z),*/
/*        transform);*/
/*    Vector3 finalPosition = {0, 0, 0};*/
/*    finalPosition         = Vector3Transform(finalPosition, transform);*/
/**/
/*    Vector2 world_pos_right*/
/*        = GetWorldToScreen(finalPosition, *next_props->camera);*/
/**/
/*    return world_pos_right.x > 0 && world_pos_right.y > 0*/
/*           && world_pos.x < GetScreenWidth() && world_pos.y <
 * GetScreenHeight();*/
/*}   */

static Vector3 text_measure(Font font, const char *text, float fontSize,
                            float fontSpacing, float lineSpacing) {
  int len = TextLength(text);
  int tempLen = 0; // Used to count longer text line num chars
  int lenCounter = 0;

  float tempTextWidth = 0.0f; // Used to count longer text line width

  float scale = fontSize / (float)font.baseSize;
  float textHeight = scale;
  float textWidth = 0.0f;

  int letter = 0; // Current character
  int index = 0;  // Index position in sprite font

  for (int i = 0; i < len; i++) {
    lenCounter++;

    int next = 0;
    letter = GetCodepoint(&text[i], &next);
    index = GetGlyphIndex(font, letter);

    // NOTE: normally we exit the decoding sequence as soon as a bad byte is
    // found (and return 0x3f) but we need to draw all of the bad bytes
    // using the '?' symbol so to not skip any we set next = 1
    if (letter == 0x3f)
      next = 1;
    i += next - 1;

    if (letter != '\n') {
      if (font.glyphs[index].advanceX != 0)
        textWidth +=
            (font.glyphs[index].advanceX) / (float)font.baseSize * scale;
      else
        textWidth += (font.recs[index].width + font.glyphs[index].offsetX) /
                     (float)font.baseSize * scale;
    } else {
      if (tempTextWidth < textWidth)
        tempTextWidth = textWidth;
      lenCounter = 0;
      textWidth = 0.0f;
      textHeight += scale + lineSpacing / (float)font.baseSize * scale;
    }

    if (tempLen < lenCounter)
      tempLen = lenCounter;
  }

  if (tempTextWidth < textWidth)
    tempTextWidth = textWidth;

  Vector3 vec = {0};
  vec.x = tempTextWidth +
          (float)((tempLen - 1) * fontSpacing / (float)font.baseSize *
                  scale); // Adds chars spacing to measure
  vec.y = 0.25f;
  vec.z = textHeight;

  return vec;
}
