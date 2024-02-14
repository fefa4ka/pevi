#pragma once

#include <eer.h>
#include <raylib.h>

#define Cursor_new(instance)    eer(Cursor, instance)
#define Cursor(instance, props) eer_withprops(Cursor, instance, _(props))

typedef struct {
    bool is_visible;
    Vector3 pos;
    Vector3 size;
} Cursor_props_t;

typedef struct {
    Vector3 pos;
    Vector3 size;
} Cursor_state_t;

eer_header(Cursor);
