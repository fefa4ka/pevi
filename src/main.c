
#include "buffer.h"
#include "camera.h"
#include "command.h"
#include "config.h"
#include "core.h"
#include "font.h"
#include "input.h"
#include "lr.h"
#include "lr_file.h"
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

InputHandler_t input_handler;

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

void handle_edit_input(InputEvent_t *event) {
  if (event->source_type == INPUT_SOURCE_KEYBOARD) {
    if (event->key_type == INPUT_KEY_CHAR) {
      int key = event->key_code;
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
    } else if (event->key_type == INPUT_KEY_SPECIAL) {
      if (event->key_code == KEY_BACKSPACE) {
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
      } else if (event->key_code == KEY_ENTER) {
        // Split the current line at cursor position
        lr_file_split(&phantom.buffer->lr, phantom.cursor.line_no, phantom.cursor.pos);
        
        // Move cursor to the beginning of the new line
        phantom.cursor.line_no++;
        phantom.cursor.pos = 1;
        
        // Update cursor needle to point to the beginning of the new line
        phantom.cursor.owner = lr_owner_find(&phantom.buffer->lr, lr_owner(phantom.cursor.line_no));
        phantom.cursor.needle = lr_owner_head(&phantom.buffer->lr, phantom.cursor.owner);
        phantom.cursor.is_eof = false;
        
        printf("Line split. Cursor: %lu.%lu\n", phantom.cursor.line_no, phantom.cursor.pos);
      }
    }
  }
}

void render(Camera_t *camera, render_body_fn Render3DBody,
            render_body_fn Render2DBody) {
  while (!pevi.is_closed) {
    input_process(&pevi, camera, &input_handler);
    
    // Handle edit mode input
    if (pevi.mode == PEVI_MODE_EDIT) {
      handle_edit_input(&input_handler.current_event);
    }
    
    render_frame(&pevi, camera->camera, Render3DBody, Render2DBody);
  }
}


void resource_load() {
  // Try to load font, fallback to default if not found
  font_default = font_load("assets/fonts/font.ttf", "assets/fonts/sdf.fs");
  
  // If font loading failed, use a default font
  if (font_default.face.texture.id == 0) {
    printf("Warning: Failed to load custom font, using default font\n");
    font_default.face = GetFontDefault();
    font_default.shader = LoadShader(0, 0); // Use default shader
  }
  
  plane = camera_plane(&camera.camera);
  phantom.font.font = font_default;
  phantom.plane = plane;
  
  // Create a test file if it doesn't exist
  FILE *test_file = fopen("test.txt", "r");
  if (test_file == NULL) {
    test_file = fopen("test.txt", "w");
    if (test_file != NULL) {
      fprintf(test_file, "Welcome to Pevi!\nThis is a test file.\nYou can edit this text in 3D space.");
      fclose(test_file);
    }
  } else {
    fclose(test_file);
  }
  
  phantom.buffer = buffer_open("test.txt");
}

int main(void) {
  window_open(&window);
  
  // Initialize core components
  core_init();
  
  // Initialize input handler
  input_init(&input_handler);
  
  resource_load();

  render(&camera, phantom_test, render_ui);

  window_close(&window);
  return 0;
}
