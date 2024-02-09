#pragma once

#include <eer.h>
#include <lr.h>
#include <raylib.h>

#define Symbol_new(instance)    eer(Symbol, instance)
#define Symbol(instance, props) eer_withprops(Symbol, instance, _(props))

typedef struct {
    Font font;
    Color tint;
    char    *content;
    lr_owner_t owner;
    size_t content_index;
    float   font_size;
    bool backface;
    Vector3 pos;
    Vector3 absolute_pos;
    bool is_selected;
    Camera *camera;
    struct {
        void (*hover)(eer_t *instance);
        void (*click)(eer_t *instance);
    } on;
} Symbol_props_t;

typedef struct {
	int codepoint;
	int glyph;
    Vector3 pos;
    Vector3 size;
    float scale;
} Symbol_state_t;

eer_header(Symbol);
