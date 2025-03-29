#include "text.h"
#include "helpers.h"
#include "input.h"
#include "logger.h"
#include "raylib.h"
#include "error.h"
#include <raymath.h>
#include <rlgl.h>
#include <stdio.h>
#include <stdlib.h>

// Draws the text background if hovered and returns whether the text is hovered.
static bool text_background_draw(const Vector4 *size, const Vector3 *size_3d,
                                 Camera_t *camera, Plane *plane) {
  BoundingBox text_box = {{0, 0, 0}, *size_3d};
  bool text_is_hovered = object_is_hovered(camera, text_box, plane);
  if (text_is_hovered) {
    Vector3 cube_center = {size->x / 2, -0.15f, size->z / 2};
    DrawCubeV(cube_center, *size_3d, BLUE);
  }
  return text_is_hovered;
}

// Iterates over each character in the content and draws the corresponding
// glyph. It updates the current drawing position and handles newline/tab cases.
// If the text is hovered, it checks for per-symbol hover and interaction.
// Returns true if any symbol is hovered.
static bool text_glyphs_draw(const char *content, Font *font, float font_size,
                             float spacing, float scale, size_t length,
                             Camera_t *camera, Plane *plane,
                             InputEvent_t *event, bool text_is_hovered) {
  Vector3 pos = {0, 0, 0};
  bool symbol_is_hovered = false;

  for (size_t index = 0; index < length; index++) {
    Glyph_t glyph = symbol_glyph(content[index], font, font_size);

    if (content[index] == '\n') {
      pos.z += glyph.size.z;
      pos.x = 0;
      continue;
    }
    if (content[index] == '\t') {
      pos.x += spacing * 2;
      continue;
    }

    // Draw the symbol.
    symbol_draw(&glyph, pos, WHITE, false);

    // If text is hovered, check if the current symbol is hovered.
    if (text_is_hovered) {
      BoundingBox symbol_box = {pos, Vector3Add(pos, glyph.size)};
      if (object_is_hovered(camera, symbol_box, plane)) {
        symbol_is_hovered = true;
        
        // Enable depth test temporarily to ensure proper rendering of hover effect
        // Only show hover effect in non-edit mode
        extern Pevi_t pevi;
        if (pevi.mode != PEVI_MODE_EDIT) {
          rlDisableDepthTest();
          DrawCubeWiresV(
              (Vector3){pos.x + glyph.size.x / 2, 0, pos.z + glyph.size.z / 2},
              glyph.size, RED);
          rlEnableDepthTest();
        }
        
        // Handle interaction
        event->source_type = INPUT_SOURCE_SYMBOL;
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          event->mouse = INPUT_MOUSE_CLICK;
          LOG_DEBUG("CLICK symbol %c", content[index]);
          
          // If a symbol is clicked, set the containing phantom as active
          if (event->mouse == INPUT_MOUSE_CLICK) {
            // The phantom will be set as active in the phantom_draw function
            // by storing the event information
            event->source = plane; // Store the plane to identify the phantom
          }
        } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
          event->mouse = INPUT_MOUSE_DRAG;
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
          event->mouse = INPUT_MOUSE_RELEASE;
        }
      }
    }

    pos.x += glyph.size.x + (spacing / (float)font->baseSize) * scale;
  }
  return symbol_is_hovered;
}

// Handles input events when text is hovered (and no symbol is individually
// hovered).
static void text_event_handle(bool symbol_is_hovered, bool text_is_hovered,
                              InputEvent_t *event) {
  if (!symbol_is_hovered && text_is_hovered) {
    event->source_type = INPUT_SOURCE_TEXT;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      event->mouse = INPUT_MOUSE_CLICK;
      LOG_DEBUG("CLICK text");
    } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      event->mouse = INPUT_MOUSE_DRAG;
      LOG_DEBUG("DRAG text");
    } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      event->mouse = INPUT_MOUSE_RELEASE;
      LOG_DEBUG("RELEASE text");
    }
  }
}

bool text_draw(FontSettings_t *settings, char *content, Plane *plane,
               Camera_t *camera, InputEvent_t *event) {
  if (!settings || !content || !plane || !camera || !event) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null parameter in text_draw");
    return false;
  }

  Font font = settings->font.face;
  float scale = settings->font_size / (float)font.baseSize;
  Vector4 size = text_measure(font, content, settings->font_size,
                              settings->spacing, settings->line_spacing);
  Vector3 size_3d =
      *(Vector3 *)&size; // Assumes that size.x, size.y, size.z form a Vector3.
  size_t length = (size_t)size.w; // 'w' holds the number of codepoints.

  // Draw text background and check for text hover.
  bool text_is_hovered = text_background_draw(&size, &size_3d, camera, plane);

  BeginShaderMode(settings->font.shader); // Activate SDF font shader.
  bool symbol_is_hovered =
      text_glyphs_draw(content, &font, settings->font_size, settings->spacing,
                       scale, length, camera, plane, event, text_is_hovered);
  EndShaderMode(); // Revert to the default shader.

  // Handle input events for text interaction.
  text_event_handle(symbol_is_hovered, text_is_hovered, event);
  
  return true;
}

// Reads the next codepoint from the text and advances the pointer.
// If a bad byte is encountered (yielding 0x3f), advances by one.
int text_codepoint_next(const char **text_ptr) {
  int next = 0;
  int codepoint = GetCodepoint(*text_ptr, &next);
  if (codepoint == 0x3f)
    next = 1;
  *text_ptr += next;
  return codepoint;
}


// Handles a newline: updates maximum metrics and resets current line counters,
// while increasing the overall text height.
static void line_newline_handle(float *line_current_width,
                                int *line_current_char_count,
                                float *line_max_width, int *line_max_char_count,
                                float *text_height, float scale,
                                float line_spacing, float font_base_size) {
  if (*line_current_width > *line_max_width)
    *line_max_width = *line_current_width;
  if (*line_current_char_count > *line_max_char_count)
    *line_max_char_count = *line_current_char_count;

  *line_current_width = 0.0f;
  *line_current_char_count = 0;
  *text_height += scale + (line_spacing / font_base_size) * scale;
}

Vector4 text_measure(Font font, const char *text, float font_size,
                     float font_spacing, float line_spacing) {
  const int text_total_length = TextLength(text);

  // Metrics for the maximum line dimensions.
  int line_max_char_count = 0;
  float line_max_width = 0.0f;

  // Metrics for the current line.
  int line_current_char_count = 0;
  float line_current_width = 0.0f;

  // Pre-cast the base size and compute the scale factor.
  const float font_base_size = (float)font.baseSize;
  const float scale = font_size / font_base_size;
  float text_height = scale; // Initial height equals one line's height.

  const char *text_ptr = text;
  while (*text_ptr != '\0') {
    int codepoint = text_codepoint_next(&text_ptr);

    // Newline: update metrics and prepare for the next line.
    if (codepoint == '\n') {
      line_newline_handle(&line_current_width, &line_current_char_count,
                          &line_max_width, &line_max_char_count, &text_height,
                          scale, line_spacing, font_base_size);
      continue;
    }

    // Process a regular character.
    line_current_char_count++;
    line_current_width +=
        font_glyph_advance_scaled_get(&font, codepoint, scale, font_base_size);

    if (line_current_char_count > line_max_char_count)
      line_max_char_count = line_current_char_count;
  }

  // Final update in case the last line is the longest.
  if (line_current_width > line_max_width)
    line_max_width = line_current_width;
  if (line_current_char_count > line_max_char_count)
    line_max_char_count = line_current_char_count;

  // Compute the measured dimensions.
  Vector4 result = {0};
  result.x = line_max_width + (((line_max_char_count - 1) * font_spacing) /
                               font_base_size * scale);
  result.y = 0.25f;
  result.z = text_height;
  result.w = text_total_length;

  return result;
}

bool text_draw_on_plane(FontSettings_t *settings, char *content, Plane *plane,
                        Camera_t *camera, InputEvent_t *event) {
  if (!settings || !content || !plane || !camera || !event) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null parameter in text_draw_on_plane");
    return false;
  }

  Vector3 pos = plane->pos;
  Vector3 angles = plane->angles;

  rlPushMatrix();
  rlTranslatef(pos.x, pos.y, pos.z);

  rlRotatef(RAD2DEG * angles.z, 0, 0, 1); // Roll (around Z-axis)
  rlRotatef(RAD2DEG * angles.y, 0, 1, 0); // Rotate around Y-axis (yaw)
  rlRotatef(RAD2DEG * angles.x, 1, 0, 0); // Rotate around X-axis (pitch)

  bool result = text_draw(settings, content, plane, camera, event);

  rlPopMatrix();
  
  return result;
}
