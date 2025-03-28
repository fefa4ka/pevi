#include "phantom.h"
#include "error.h"
#include "helpers.h"
#include "logger.h"
#include "memory.h"
#include "phantom_list.h"
#include "stdlib.h"
#include "text.h"

// For tracking the initial click offset during drag operations
static struct {
  bool initialized;
  Vector3 offset;
  Phantom_t *phantom; // Track which phantom the offset is for
} drag_offset = {false, {0}, NULL};

static struct {
  bool active;         // Is drag currently active?
  Phantom_t *phantom;  // Which phantom is being dragged
  Vector3 start_pos;   // Starting position of the phantom
  Vector2 start_mouse; // Starting position of the mouse
} drag_state = {0};

bool phantom_draw_on_plane(Phantom_t *phantom, Camera_t *camera,
                           InputEvent_t *event) {
  if (!phantom || !camera || !event) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR,
              "Null parameter in phantom_draw_on_plane");
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
  
  // Always draw the phantom background with its color, but with lower alpha if not hovered/selected
  Vector3 center = {size->x / 2, -0.15f, size->z / 2};
  
  Color bg_color = phantom->color;
  
  // Make the color more transparent if not hovered or selected
  if (!phantom_hovered && !phantom->is_selected) {
    bg_color.a = 100; // Lower alpha for non-hovered/non-selected phantoms
  } else {
    bg_color.a = 200; // Higher alpha for hovered/selected phantoms
  }
  
  DrawCubeV(center, *size_3d, bg_color);
  
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
                LOG_DEBUG("Cursor: %lu.%lu", line_no, line_pos);

                // Set this phantom as active when a symbol is clicked
                extern Pevi_t pevi;
                if (pevi.phantoms) {
                  phantom_list_set_active_by_id(pevi.phantoms, phantom->id);
                  LOG_DEBUG(
                      "Set phantom with ID %d as active due to symbol click",
                      phantom->id);
                }
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

// Calculate new position for phantom during drag operation
static Vector3 phantom_drag_position_calculate(Camera_t *camera,
                                               Vector3 current_pos) {
  LOG_DEBUG("Calculating drag position for phantom %d", 
           drag_state.phantom ? drag_state.phantom->id : -1);
  
  // Get the ray from camera to mouse position
  Ray ray = GetMouseRay(GetMousePosition(), camera->camera);
  
  // Store the initial distance from camera to phantom when drag starts
  static float initial_distance = 0.0f;
  
  // On first drag calculation, store the initial distance
  if (!drag_offset.initialized || drag_offset.phantom != drag_state.phantom) {
    initial_distance = Vector3Distance(camera->camera.position, current_pos);
    LOG_DEBUG("Initial distance to camera: %.2f", initial_distance);
  }
  
  // Create a plane that's aligned with the camera view plane
  // This is the same approach used in camera_plane()
  Vector3 camera_forward = Vector3Normalize(
      Vector3Subtract(camera->camera.target, camera->camera.position));
  
  // Create a point on the plane at the initial distance from the camera
  Vector3 plane_point = Vector3Add(
      camera->camera.position,
      Vector3Scale(camera_forward, initial_distance)
  );
  
  // The plane normal is the same as the camera forward direction
  Vector3 plane_normal = camera_forward;
  
  // Calculate intersection of ray with this plane
  float denominator = Vector3DotProduct(ray.direction, plane_normal);
  if (fabs(denominator) < 0.0001f) {
    // Ray is parallel to plane, no intersection
    return current_pos;
  }
  
  // Calculate the plane constant d from the point-normal form of the plane equation
  float d = -Vector3DotProduct(plane_normal, plane_point);
  
  // Calculate t where ray intersects the plane
  float t = -(Vector3DotProduct(plane_normal, ray.position) + d) / denominator;
  
  if (t < 0) {
    // Intersection is behind the camera
    return current_pos;
  }
  
  // Calculate intersection point
  Vector3 intersection = Vector3Add(ray.position, Vector3Scale(ray.direction, t));

  // If this is the first drag calculation for this phantom, calculate the offset
  if ((!drag_offset.initialized || drag_offset.phantom != drag_state.phantom) && drag_state.active) {
    // Calculate the offset from the intersection point to the phantom position
    drag_offset.offset = Vector3Subtract(current_pos, intersection);
    drag_offset.initialized = true;
    drag_offset.phantom = drag_state.phantom;
    
    LOG_DEBUG("Initialized drag offset=(%.2f,%.2f,%.2f) for phantom %d",
             drag_offset.offset.x, drag_offset.offset.y, drag_offset.offset.z,
             drag_state.phantom ? drag_state.phantom->id : -1);
  }

  // Apply the offset to maintain the relative position
  Vector3 adjusted_position = Vector3Add(intersection, drag_offset.offset);
  
  // Ensure we maintain the exact same distance from camera and stay on the camera plane
  Vector3 camera_to_adjusted = Vector3Subtract(adjusted_position, camera->camera.position);
  float current_distance = Vector3Length(camera_to_adjusted);
  
  // Scale to maintain the original distance
  Vector3 fixed_position = Vector3Add(
      camera->camera.position,
      Vector3Scale(Vector3Normalize(camera_to_adjusted), initial_distance)
  );
  
  // Apply some smoothing/damping to make movement less jerky
  const float smoothing = 0.8f;
  Vector3 new_pos = {
      current_pos.x + (fixed_position.x - current_pos.x) * smoothing,
      current_pos.y + (fixed_position.y - current_pos.y) * smoothing,
      current_pos.z + (fixed_position.z - current_pos.z) * smoothing};

  return new_pos;
}

// Handles input events when the phantom is hovered or being dragged
static void phantom_event_handle(Phantom_t *phantom, InputEvent_t *event,
                                 Camera_t *camera) {
  // Check if we're already dragging this phantom
  bool is_dragging = (drag_state.active && drag_state.phantom == phantom);

  // Handle events when phantom is hovered or being dragged
  if (phantom->is_hovered || is_dragging) {
    event->source_type = INPUT_SOURCE_PHANTOM;
    event->source = phantom;

    // Handle mouse click - start potential drag
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      event->mouse = INPUT_MOUSE_CLICK;
      
      // Always select the phantom on click, don't toggle
      phantom->is_selected = true;

      // Set this phantom as the active one in the phantom list
      extern Pevi_t pevi;
      if (pevi.phantoms) {
        LOG_DEBUG("Setting phantom with ID %d as active", phantom->id);
        phantom_list_set_active_by_id(pevi.phantoms, phantom->id);

        // Initialize drag state
        drag_state.active = false;
        drag_state.phantom = phantom;
        drag_state.start_pos = phantom->plane.pos;
        drag_state.start_mouse = GetMousePosition();
      }

      LOG_DEBUG("CLICK phantom %d (selected)", phantom->id);
    }
    // Handle mouse drag - continue drag if already started or start if mouse
    // moved enough
    else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && phantom->is_selected) {
      // Only reposition in free mode
      extern Pevi_t pevi;
      if (pevi.mode == PEVI_MODE_FREE) {
        Vector2 current_mouse = GetMousePosition();

        // Start drag if mouse has moved enough
        if (!drag_state.active &&
            Vector2Distance(current_mouse, drag_state.start_mouse) > 1.0f) {
          drag_state.active = true;
          
          // Reset drag offset when starting a new drag
          drag_offset.initialized = false;
          drag_offset.phantom = NULL;
          
          // Make phantom face the camera when drag starts
          // Use the camera_plane function to get proper alignment
          Plane camera_plane_alignment = camera_plane(&camera->camera);
          phantom->plane.angles = camera_plane_alignment.angles;
          
          LOG_DEBUG("Starting drag of phantom %d", phantom->id);
        }

        // Continue drag if active
        if (drag_state.active) {
          event->mouse = INPUT_MOUSE_DRAG;

          // Calculate new position based on mouse movement
          Vector3 new_pos =
              phantom_drag_position_calculate(camera, phantom->plane.pos);

          // Update phantom position
          phantom->plane.pos = new_pos;
          
          // Keep phantom aligned with camera during drag
          Plane camera_plane_alignment = camera_plane(&camera->camera);
          phantom->plane.angles = camera_plane_alignment.angles;

          LOG_DEBUG("DRAG phantom to position: %.2f, %.2f, %.2f",
                    phantom->plane.pos.x, phantom->plane.pos.y,
                    phantom->plane.pos.z);
        }
      }
    }
    // Handle mouse release - end drag
    else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      event->mouse = INPUT_MOUSE_RELEASE;

      // End drag if active
      if (drag_state.active) {
        LOG_DEBUG("Ending drag of phantom %d", phantom->id);
        drag_state.active = false;
        drag_state.phantom = NULL;
        
        // Reset drag offset
        drag_offset.initialized = false;
        drag_offset.phantom = NULL;
      }

      LOG_DEBUG("RELEASE phantom");
    }
  }

  // Special case: if we're dragging this phantom but mouse is no longer over
  // it, we still need to handle the drag
  else if (drag_state.active && drag_state.phantom == phantom &&
           IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    event->source_type = INPUT_SOURCE_PHANTOM;
    event->source = phantom;
    event->mouse = INPUT_MOUSE_DRAG;

    // Only reposition in free mode
    extern Pevi_t pevi;
    if (pevi.mode == PEVI_MODE_FREE) {
      // Calculate new position based on mouse movement
      Vector3 new_pos =
          phantom_drag_position_calculate(camera, phantom->plane.pos);

      // Update phantom position
      phantom->plane.pos = new_pos;
      
      // Keep phantom aligned with camera during drag
      Plane camera_plane_alignment = camera_plane(&camera->camera);
      phantom->plane.angles = camera_plane_alignment.angles;

      LOG_DEBUG("DRAG phantom (outside) to position: %.2f, %.2f, %.2f",
                phantom->plane.pos.x, phantom->plane.pos.y,
                phantom->plane.pos.z);
    }
  }
}

// ---------------------------------------------------------------------------
// Main Phantom Draw Function
// ---------------------------------------------------------------------------

bool phantom_draw(Phantom_t *phantom, Camera_t *camera, InputEvent_t *event) {
  if (!phantom || !camera || !event) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR,
              "Null parameter in phantom_draw");
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
  if (phantom->is_hovered || (drag_state.active && drag_state.phantom == phantom)) {
    phantom_event_handle(phantom, event, camera);
  }

  // Check if this phantom contains the plane that was clicked on a symbol
  if (event->source_type == INPUT_SOURCE_SYMBOL &&
      event->mouse == INPUT_MOUSE_CLICK && event->source == &phantom->plane) {
    // Clear the source to prevent multiple activations
    event->source = NULL;

    // Set this phantom as active
    extern Pevi_t pevi;
    if (pevi.phantoms) {
      phantom_list_set_active_by_id(pevi.phantoms, phantom->id);
      LOG_DEBUG(
          "Set phantom with ID %d as active due to symbol click (from plane)",
          phantom->id);
    }
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

Vector4 phantom_measure(const Phantom_t *phantom) {
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
    ERROR_SET(ERROR_MEMORY_ALLOCATION, ERROR_ERROR,
              "Failed to allocate phantom");
    return NULL;
  }

  // Initialize phantom fields
  phantom->id = 0;
  phantom->buffer = NULL;
  phantom->line_from = 1;
  phantom->line_to = 1;
  phantom->plane = (Plane){0};
  phantom->font = (FontSettings_t){0};
  phantom->color = color_random_pastel(); // Set a random pastel color
  phantom->cursor = (Cursor_t){0};
  phantom->is_selected = false;
  phantom->is_hovered = false;

  return phantom;
}

void phantom_free(Phantom_t *phantom) {
  if (!phantom)
    return;

  // Free the buffer if it exists
  if (phantom->buffer) {
    buffer_free(phantom->buffer);
    phantom->buffer = NULL;
  }

  // Free the phantom structure itself
  FREE(phantom);
}
