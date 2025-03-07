
#include "buffer.h"
#include "camera.h"
#include "command.h"
#include "config.h"
#include "core.h"
#include "font.h"
#include "input.h"
#include "lr.h"
#include "raylib.h"
#include "render.h"
#include "text.h"
#include "window.h"
#include <stdio.h>

Window_t window = {WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, FPS_TARGET};

Pevi_t pevi = {PEVI_MODE_FREE, true};

Camera_t camera = {
    {CAMERA_POSITION, CAMERA_TARGET, CAMERA_UP, CAMERA_FOVY, CAMERA_PROJECTION},
    CAMERA_MODE,
    true,
    false,
    true};

Font_t font_default;

Plane plane;
Phantom_t phantom = {
    .font = {.font_size = 16, .spacing = 0.6, .line_spacing = 0.6},
    .line_from = 1,
    .line_to = 3};

void quit(void *arg) { pevi.is_closed = true; }
Command_t commands[] = {
    {"q", quit, &pevi.command_buffer}, {0} // End marker
};

void phantom_test(Pevi_t *pevi) {
  InputEvent_t event = {0};
  phantom_draw_on_plane(&phantom, &camera, &event);
  DrawGrid(100, 1.0f);
}

void render(Camera_t *camera, render_body_fn Render3DBody,
            render_body_fn Render2DBody) {
  while (!pevi.is_closed) {
    input_process(&pevi, camera);
    render_frame(&pevi, camera->camera, Render3DBody, Render2DBody);
    if (pevi.mode == PEVI_MODE_EDIT) {

      int key = GetCharPressed();
      if (key) {
        if (phantom.cursor.pos == 1) {
          lr_insert(&phantom.buffer->lr, key, phantom.cursor.line_no,
                    phantom.cursor.pos);
          phantom.cursor.needle = phantom.cursor.needle->next;
          phantom.cursor.pos++;
        } else if (phantom.cursor.is_eof) {
          lr_put(&phantom.buffer->lr, key, phantom.cursor.line_no);
          phantom.cursor.needle = lr_owner_tail(phantom.cursor.owner);
          phantom.cursor.pos++;
        } else {
          lr_insert_next(&phantom.buffer->lr, key, phantom.cursor.needle);
          phantom.cursor.pos++;
          phantom.cursor.needle = phantom.cursor.needle->next;
        }
      }
      if (IsKeyPressed(KEY_BACKSPACE)) {
        if (phantom.cursor.is_eof) {
          lr_pop(&phantom.buffer->lr, &phantom.cursor.needle->data,
                 phantom.cursor.line_no);
          phantom.cursor.pos--;
          phantom.cursor.needle = lr_owner_tail(phantom.cursor.owner);
        } else {
          lr_pull(&phantom.buffer->lr, &phantom.cursor.needle->data,
                  phantom.cursor.line_no, phantom.cursor.pos);
          phantom.cursor.pos--;

          phantom.cursor.needle = NULL;
        }
        printf("Cursor: %lu.%lu\n", phantom.cursor.line_no, phantom.cursor.pos);
      }
    }
  }
}


void resource_load() {
  font_default = font_load("lego/font/font.ttf", "lego/font/sdf.fs");
  plane = camera_plane(&camera.camera);
  phantom.font.font = font_default;
  phantom.plane = plane;
  phantom.buffer = buffer_open("test.txt");
}

int main(void) {
  window_open(&window);

  resource_load();

  render(&camera, phantom_test, render_ui);

  window_close(&window);
  return 0;
}
