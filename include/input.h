#pragma once
#include "camera.h"

typedef enum {
  INPUT_SOURCE_NONE,
  INPUT_SOURCE_SYMBOL,
  INPUT_SOURCE_TEXT,
  INPUT_SOURCE_PHANTOM,
  INPUT_SOURCE_SPACE
} InputSource_t;

typedef enum {
  INPUT_MOUSE_NONE,
  INPUT_MOUSE_CLICK,
  INPUT_MOUSE_DRAG,
  INPUT_MOUSE_RELEASE
} InputMouse_t;

typedef struct {
  InputSource_t source_type;
  void *source;

  KeyboardKey keyboard;
  InputMouse_t mouse;

  // Define callbacks for hover, click, drag, release, etc
  struct {
    void (*hover)(void *source);
    void (*click)(void *source);
    void (*drag)(void *source);
    void (*release)(void *source);
  } on;
} InputEvent_t;

void input_process(Pevi_t *pevi, Camera_t *camera);
