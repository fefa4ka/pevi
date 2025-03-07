#include "phantom.h"
#include "helpers.h"
#include "lr_file.h"
#include "stdlib.h"
#include "text.h"
#include "error.h"

bool phantom_draw_on_plane(Phantom_t *phantom, Camera_t *camera,
                           InputEvent_t *event) {
  if (!phantom || !camera || !event) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null parameter in phantom_draw_on_plane");
    return false;
  }

  Vector3 pos = phantom->plane.pos;
  Vector3 angles = phantom->plane.angles;

  rlPushMatrix();
  rlTranslatef(pos.x, pos.y, pos.z);

  rlRotatef(RAD2DEG * angles.z, 0, 0, 1); // Roll (around Z-axis)
  rlRotatef(RAD2DEG * angles.y, 0, 1, 0); // Yaw (around Y-axis)
  rlRotatef(RAD2DEG * angles.x, 1, 0, 0); // Pitch (around X-axis)

  bool result = phantom_draw(phantom, camera, event);

  rlPopMatrix();
  
  return result;
}

// Draws the phantom background if hovered and returns whether it is hovered.
static void phantom_draw_background(const Vector4 *size, const Vector3 *size_3d,
                                    Phantom_t *phantom, Camera_t *camera) {
  BoundingBox phantom_box = {{0, 0, 0}, *size_3d};
  bool phantom_hovered =
      object_is_hovered(camera, phantom_box, &phantom->plane);
  if (phantom_hovered || phantom->is_selected) {
    Vector3 center = {size->x / 2, -0.15f, size->z / 2};
    DrawCubeV(center, *size_3d, BLUE);
  }
  phantom->is_hovered = phantom_hovered;
}

// Draws each line in the phantom's buffer.
// Iterates from `line_from` to `line_to` and, for each line, loops through its
// lr_cells.
static void phantom_draw_lines(Phantom_t *phantom, const Font *font,
                               float font_size, float spacing, float scale,
                               Camera_t *camera, Plane *plane,
                               InputEvent_t *event) {
  Vector3 pos = {0, 0, 0};
  bool symbol_is_hovered = false;

  // For each line in the phantom.
  for (size_t line_no = phantom->line_from; line_no <= phantom->line_to;
       line_no++) {
    // Retrieve the line container.
    size_t line_pos = 0;
    struct lr_cell *line =
        lr_owner_find(&phantom->buffer->lr, lr_owner(line_no));
    if (!line)
      continue;

    // Get the head and tail of the line.
    struct lr_cell *cell_head = lr_owner_head(&phantom->buffer->lr, line);
    struct lr_cell *cell_tail = lr_owner_tail(line);

    // Iterate through the circular list.
    struct lr_cell *needle = cell_head;
    do {
      char ch = needle->data;
      line_pos++;

      // Handle tab character.
      if (ch == '\t') {
        pos.x += spacing * 2;
        continue;
      }

      if (phantom->cursor.needle == NULL &&
          phantom->cursor.line_no == line_no &&
          phantom->cursor.pos == line_pos) {
        phantom->cursor.needle = needle;
      }
      // Draw the glyph.
      Glyph_t glyph = symbol_glyph(ch, font, font_size);
      if (phantom->is_hovered) {
        BoundingBox symbol_box = {pos, Vector3Add(pos, glyph.size)};
        if (object_is_hovered(camera, symbol_box, plane)) {
          symbol_is_hovered = true;
          if (event->source_type == INPUT_SOURCE_NONE) {
            symbol_interaction_handle(ch, pos, glyph.size, event);
            if (event->source_type == INPUT_SOURCE_SYMBOL) {
              if (event->mouse == INPUT_MOUSE_CLICK) {
                phantom->cursor.owner = line;
                phantom->cursor.needle = needle;
                phantom->cursor.line_no = line_no;
                phantom->cursor.pos = line_pos;
                if (needle == cell_tail) {
                  phantom->cursor.is_eof = true;
                } else {
                  phantom->cursor.is_eof = false;
                }
                printf("Cursor: %lu.%lu\n", line_no, line_pos);
              }
            }
          }
        }
      }
      // Draw cursor
      if (phantom->cursor.needle && phantom->cursor.needle == needle) {
        Vector3 cursor_size = glyph.size;
        cursor_size.y = 0;
        DrawCubeV(
            (Vector3){pos.x + glyph.size.x / 2, 0, pos.z + glyph.size.z / 2},
            cursor_size, RED);
      }

      symbol_draw(&glyph, pos, WHITE, false);

      // Advance the x position by glyph width plus additional spacing.
      pos.x += glyph.size.x + (spacing / (float)font->baseSize) * scale;
      needle = needle->next;
    } while (needle != cell_tail->next);

    // End of line: reset x and advance z by line height.
    pos.x = 0;
    pos.z +=
        scale + (phantom->font.line_spacing / (float)font->baseSize) * scale;
  }
}

// Handles input events when the phantom is hovered.
static void phantom_event_handle(Phantom_t *phantom, InputEvent_t *event) {
  if (phantom->is_hovered && event->source_type != INPUT_SOURCE_SYMBOL) {
    event->source_type =
        INPUT_SOURCE_PHANTOM; // (Assuming this constant exists.)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      event->mouse = INPUT_MOUSE_CLICK;
      phantom->is_selected = !phantom->is_selected;
      printf("CLICK phantom %d\n", phantom->is_selected);
    } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      event->mouse = INPUT_MOUSE_DRAG;
      printf("DRAG phantom\n");
    } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      event->mouse = INPUT_MOUSE_RELEASE;
      printf("RELEASE phantom\n");
    }
  }
}

// ---------------------------------------------------------------------------
// Main Phantom Draw Function
// ---------------------------------------------------------------------------

bool phantom_draw(Phantom_t *phantom, Camera_t *camera, InputEvent_t *event) {
  if (!phantom || !camera || !event) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null parameter in phantom_draw");
    return false;
  }
  
  if (!phantom->buffer) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Phantom has no buffer");
    return false;
  }

  // Extract the phantom's font settings.
  FontSettings_t settings = phantom->font;
  Font font = settings.font.face;
  float scale = settings.font_size / (float)font.baseSize;

  // Update line_to to show all lines in the buffer
  /*size_t line_count = lr_owner_count(&phantom->buffer->lr);*/
  /*if (line_count > phantom->line_to) {*/
  /*  phantom->line_to = line_count;*/
  /*}*/

  // Measure the phantom's dimensions.
  Vector4 size = phantom_measure(phantom);
  Vector3 size_3d =
      *(Vector3 *)&size; // Assumes size.x, size.y, size.z form a Vector3.

  // Draw the phantom background and determine if it is hovered.
  phantom_draw_background(&size, &size_3d, phantom, camera);

  // Activate the shader for font rendering.
  BeginShaderMode(settings.font.shader);
  // Draw each line (i.e. each row of text) from the phantom's buffer.
  phantom_draw_lines(phantom, &font, settings.font_size, settings.spacing,
                     scale, camera, &phantom->plane, event);
  EndShaderMode();

  // Handle input events for the phantom.
  if (phantom->is_hovered) {
    phantom_event_handle(phantom, event);
  }
  
  return true;
}

// ---------------------------------------------------------------------------
// Phantom Buffer Helpers
// ---------------------------------------------------------------------------

// Measures a single line in the phantom's buffer by iterating over its
// lr_cells. Assumes each cell stores a character in the field `data`.
static phantom_line_measure_t
phantom_line_measure_get(const Buffer_t *buffer, size_t line_no,
                         const Font *face, float scale, float font_base_size) {
  phantom_line_measure_t measure = {0, 0.0f};

  // Get the line container (lr_cell) for the requested line.
  struct lr_cell *line = lr_owner_find(&buffer->lr, lr_owner(line_no));
  if (!line)
    return measure;

  // Get the head and tail of the line.
  struct lr_cell *needle = lr_owner_head(&buffer->lr, line);
  struct lr_cell *tail = lr_owner_tail(line);

  // Loop through the circular list.
  do {
    measure.char_count++;

    // Assume the cell stores a character in `data`.
    char ch = needle->data;
    int codepoint = (unsigned char)ch;
    measure.width +=
        font_glyph_advance_scaled_get(face, codepoint, scale, font_base_size);

    needle = needle->next;
  } while (needle != tail->next);

  return measure;
}

// ---------------------------------------------------------------------------
// Phantom Measure Function
// ---------------------------------------------------------------------------

static Vector4 phantom_measure(const Phantom_t *phantom) {
  // Extract font settings.
  FontSettings_t settings = phantom->font;
  Font_t font_wrapper = settings.font;
  Font face = font_wrapper.face;
  const float font_base_size = (float)face.baseSize;
  const float scale = settings.font_size / font_base_size;

  int total_char_count = 0;
  float line_max_width = 0.0f;
  int line_max_char_count = 0;

  // Initial text height equals one line's height.
  float text_height = scale;

  // Iterate over each line from line_from to line_to.
  for (size_t line_no = phantom->line_from; line_no <= phantom->line_to;
       line_no++) {
    phantom_line_measure_t measure = phantom_line_measure_get(
        phantom->buffer, line_no, &face, scale, font_base_size);
    total_char_count += measure.char_count;
    if (measure.width > line_max_width)
      line_max_width = measure.width;
    if (measure.char_count > line_max_char_count)
      line_max_char_count = measure.char_count;

    // Add spacing for line breaks (except after the last line).
    if (line_no < phantom->line_to)
      text_height += scale + (settings.line_spacing / font_base_size) * scale;
  }

  // Compute final measured dimensions.
  Vector4 result = {0};
  result.x =
      line_max_width + (((line_max_char_count - 1) * settings.line_spacing) /
                        font_base_size * scale);
  result.y = 0.25f;
  result.z = text_height;
  result.w = total_char_count + phantom->line_to - phantom->line_from;

  return result;
}

// ---------------------------------------------------------------------------
// Phantom Creation and Cleanup
// ---------------------------------------------------------------------------

Phantom_t *phantom_create(void) {
  Phantom_t *phantom = MALLOC(sizeof(Phantom_t));
  if (!phantom) {
    ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR, "Failed to allocate phantom");
    return NULL;
  }
  
  // Initialize phantom fields
  phantom->id = 0;
  phantom->buffer = NULL;
  phantom->line_from = 1;
  phantom->line_to = 1;
  phantom->plane = (Plane){0};
  phantom->font = (FontSettings_t){0};
  phantom->cursor = (Cursor_t){0};
  phantom->is_selected = false;
  phantom->is_hovered = false;
  
  return phantom;
}

void phantom_free(Phantom_t *phantom) {
  if (!phantom) return;
  
  // Free the buffer if it exists
  if (phantom->buffer) {
    buffer_free(phantom->buffer);
    phantom->buffer = NULL;
  }
  
  // Free the phantom structure itself
  FREE(phantom);
}
