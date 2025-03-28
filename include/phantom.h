#pragma once
#include "buffer.h"
#include "camera.h"
#include "core.h"
#include "font.h"
#include "input.h"

typedef struct {
  size_t line_no;
  size_t pos;
  bool is_eof;
  struct lr_cell *owner;
  struct lr_cell *needle;
} Cursor_t;

typedef struct {
  int id;
  Buffer_t *buffer;

  // TODO: Add features for extracting text from buffer
  // to phantom (e.g. struct, function, etc.)
  size_t line_from;
  size_t line_to;

  Plane plane;

  FontSettings_t font;
  Color color;  // Color for the phantom

  Cursor_t cursor;
  bool is_selected;
  bool is_hovered;
} Phantom_t;

typedef struct {
    int char_count;
    float width;
} phantom_line_measure_t;

bool phantom_draw(Phantom_t *phantom, Camera_t *camera, InputEvent_t *event);
bool phantom_draw_on_plane(Phantom_t *phantom, Camera_t *camera,
                           InputEvent_t *event);
Phantom_t *phantom_create(void);
Vector4 phantom_measure(const Phantom_t *phantom);
void phantom_free(Phantom_t *phantom);
