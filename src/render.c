#include "render.h"
#include "text.h"


void render_frame(Pevi_t *pevi, Camera3D camera, render_body_fn Render3DBody,
                  render_body_fn Render2DBody) {

  BeginDrawing();

  BeginMode3D(camera);
  ClearBackground(BLANK);
  rlEnableDepthTest();

  Render3DBody(pevi);

  EndMode3D();

  Render2DBody(pevi);

  EndDrawing();
}

void render_status_bar(Pevi_t *pevi) {
  char path[] = "STATUS BAR";
  if (pevi->is_fps_visible) {
    DrawFPS(10, 10);
  }
  char *bar_content = path;

  if(pevi->mode == PEVI_MODE_COMMAND) {
	  bar_content = pevi->command_buffer.buffer;
  }
  DrawText(bar_content, 10, GetScreenHeight() - 70, 30, WHITE);
}

void render_ui(Pevi_t *pevi) { render_status_bar(pevi); }

void render_phantom(Phantom_t *phantom, Camera_t *camera, InputEvent_t *event) {
  phantom_draw_on_plane(phantom, camera, event);
}
