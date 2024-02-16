#pragma once

#include <eer.h>
#include <raylib.h>
#include <stdio.h>
#include <unistd.h>

#define Terminal_new(instance)    eer(Terminal, instance)
#define Terminal(instance, props) eer_withprops(Terminal, instance, _(props))

typedef struct {
    char   *command;
    bool is_visible;
    Camera *camera;
    Vector3 pos;
    Vector2 angles;
    Vector3 size;
    Color bg_color;
} Terminal_props_t;

typedef struct {
    FILE *fp;
fd_set fds;
int    fd;

    char  *content;
    char  buffer[1024];
    size_t content_size;
    Vector3 pos;
    Vector3 size;
} Terminal_state_t;

eer_header(Terminal);
