#pragma once

#include <eer.h>
#include <lr.h>
#include <raylib.h>

#define Symbol_new(instance)    eer(Symbol, instance)
#define Symbol(instance, props) eer_withprops(Symbol, instance, _(props))

typedef struct {
    Font font;
    Shader shader;
    Color tint;
    char *content;
    void *parent;
    lr_owner_t owner;
    void *parent_arg;
    Vector3 pos;
    Vector2 angles;
    Vector3 absolute_pos;
    float   font_size;
    bool backface;
    bool is_selected;
    bool is_hovered;
    Camera *camera;
    struct {
        void (*hover)(eer_t *instance);
        void (*click)(eer_t *instance);
    } on;
} Symbol_props_t;

typedef struct {
    Vector3 pos;
    Vector3 size;
    float scale;
	int codepoint;
	int glyph;
} Symbol_state_t;

eer_header(Symbol);
