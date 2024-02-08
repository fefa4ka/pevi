#pragma once

#include <eer.h>
#include <raylib.h>

#define Symbol_new(instance)    eer(Symbol, instance)
#define Symbol(instance, props) eer_withprops(Symbol, instance, _(props))

typedef struct {
    Font font;
    Color tint;
    char    *content;
    float   font_size;
    bool backface;
    Vector3 pos;
    Vector3 absolute_pos;
    bool is_selected;
    Camera *camera;
} Symbol_props_t;

typedef struct {
	int codepoint;
	int glyph;
    Vector3 pos;
    Vector3 size;
    float scale;
} Symbol_state_t;

eer_header(Symbol);
