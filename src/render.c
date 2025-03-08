#include "render.h"
#include "text.h"
#include "error.h"
#include "phantom.h"
#include <string.h>
#include "phantom_list.h"


bool render_frame(Pevi_t *pevi, Camera3D camera, render_body_fn Render3DBody,
                  render_body_fn Render2DBody) {
  if (!pevi || !Render3DBody || !Render2DBody) {
    ERROR_SET(ERROR_INVALID_PARAMETER, ERROR_ERROR, "Null parameter in render_frame");
    return false;
  }

  BeginDrawing();

  BeginMode3D(camera);
  ClearBackground(BLANK);
  rlEnableDepthTest();

  // Call the 3D rendering function
  Render3DBody(pevi);

  EndMode3D();

  // Call the 2D rendering function
  Render2DBody(pevi);

  EndDrawing();
  
  return true;
}

void render_status_bar(Pevi_t *pevi) {
  char mode_str[32] = {0};
  
  // Show current mode
  switch(pevi->mode) {
    case PEVI_MODE_FREE:
      strcpy(mode_str, "FREE");
      break;
    case PEVI_MODE_EDIT:
      strcpy(mode_str, "EDIT");
      break;
    case PEVI_MODE_COMMAND:
      strcpy(mode_str, "COMMAND");
      break;
    default:
      strcpy(mode_str, "UNKNOWN");
      break;
  }
  
  if (pevi->is_fps_visible) {
    DrawFPS(10, 10);
  }
  
  // Show active phantom info if available
  Phantom_t *active = phantom_list_get_active(pevi->phantoms);
  char status_text[256] = {0};
  
  if (active && active->buffer && active->buffer->path) {
    snprintf(status_text, sizeof(status_text), "[%s] %s", mode_str, active->buffer->path);
  } else {
    snprintf(status_text, sizeof(status_text), "[%s]", mode_str);
  }
  
  // In command mode, show the command buffer
  if(pevi->mode == PEVI_MODE_COMMAND) {
    snprintf(status_text, sizeof(status_text), ":%s", pevi->command_buffer.buffer);
  }
  
  DrawText(status_text, 10, GetScreenHeight() - 70, 30, WHITE);
}

void render_ui(Pevi_t *pevi) { render_status_bar(pevi); }

void render_phantom(Phantom_t *phantom, Camera_t *camera, InputEvent_t *event) {
  phantom_draw_on_plane(phantom, camera, event);
}
