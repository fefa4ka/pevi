#include "input.h"

#include "command.h"
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

static int key_pressed = 0;

bool input_key_handler();
int input_key_get();
void input_process(Pevi_t *pevi, Camera_t *camera) {
  camera_handle(camera);
  // TODO: Handle keyboard and mouse input
  // For now, you could check for a key press and print a message.
  if (IsKeyPressed(KEY_SPACE)) {
    // Placeholder action for spacebar press
    printf("Space key pressed!\n");
  }
  if (pevi->mode == PEVI_MODE_FREE) {
    int key = GetCharPressed();
    if (key == 'e') {
      printf("Edit mode\n");
      pevi->mode = PEVI_MODE_EDIT;
      camera->is_enabled = false;
      camera->camera.projection = CAMERA_ORTHOGRAPHIC;
    } else if (key == ':') {
      printf("Command mode\n");
      pevi->mode = PEVI_MODE_COMMAND;
      camera->is_enabled = false;
    }
  } else if (pevi->mode == PEVI_MODE_COMMAND) {
    command_buffer_update(&pevi->command_buffer);
  }
  if (pevi->mode != PEVI_MODE_FREE) {
    if (IsKeyPressed(KEY_ESCAPE)) {
      printf("Free mode\n");
      pevi->mode = PEVI_MODE_FREE;
      camera->is_enabled = true;
      camera->camera.projection = CAMERA_PERSPECTIVE;
    }
  }
}

bool input_key_handler() {
  if (key_pressed) {
    return true;
  }

  if (IsKeyPressed(KEY_ENTER)) {
    key_pressed = '\r';
    return true;
  }

  if (IsKeyPressed(KEY_BACKSPACE)) {
    key_pressed = '\b';
    return true;
  }
  key_pressed = GetCharPressed();
  if (key_pressed) {
    return true;
  }
  return false;
}

int input_key_get() {
  int key = key_pressed;
  key_pressed = 0;
  return key;
}
