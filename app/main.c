
#include "camera.h"
#include "config.h"
#include "core.h"
#include "font.h"
#include "raylib.h"
#include "render.h"
#include "symbol.h"
#include "window.h"
#include "buffer.h"
#include <stdio.h>

Window_t window = {WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, FPS_TARGET};

Camera_t camera = {
    {CAMERA_POSITION, CAMERA_TARGET, CAMERA_UP, CAMERA_FOVY, CAMERA_PROJECTION},
    CAMERA_MODE,
    true,
    false,
    true};

Font_t font_default;

void status_bar() {
  char path[] = "STATUS BAR";
  DrawFPS(10, 10);
  DrawText(path, 10, GetScreenHeight() - 70, 30, WHITE);
}

  Plane plane;
void render_test() {
  FontSettings settings = {.font = font_default,
                           .font_size = 16,
                           .spacing = 0.6,
                           .line_spacing = 0.6};
  float fontSize = 16.0f;
  Vector3 fontPosition = {0, 0, 0};
  char *content = "Hello\nworld\0";

  text_draw_on_plane(&settings, content, &plane, &camera);
  DrawGrid(100, 1.0f);
}

void render(Camera_t *camera, render_body_fn Render3DBody,
            render_body_fn Render2DBody) {
  while (!WindowShouldClose()) {
    camera_handle(camera);
    render_frame(camera->camera, Render3DBody, Render2DBody);
  }
}

int main(void) {
  window_open(&window);

  Buffer_t *buffer = file_open("test.txt");
  plane = camera_plane(&camera.camera);

  lr_dump(&buffer->lr);
  font_default = font_load("lego/font/font.ttf", "lego/font/sdf.fs");

  render(&camera, render_test, status_bar);

  window_close(&window);
  return 0;
}
