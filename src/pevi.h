#pragma once

#include <Camera.h>
#include <Clock.h>
#include <Cursor.h>
#include <Menu.h>
#include <Serial.h>
#include <Symbol.h>
#include <Text.h>
#include <Window.h>
#include <eer_app.h>
#include <lr.h>
#include <raylib.h>
#include <raymath.h>
#include <rcamera.h>
#include <rlgl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cfg.h"
#include "cmd.h"
#include "input.h"
#include "pemath.h"

enum pevi_mode { PEVI_MODE_FREE, PEVI_MODE_EDIT, PEVI_MODE_COMMAND, PEVI_MODE_DRAG };

struct Plane {
    Vector3 pos;
    Vector2 angles;
    Color   tint;
};

struct PeviCursor {
    struct Plane       *plane;
    struct linked_ring *buffer; // Text buffer
    struct lr_cell     *cell;   // selected cell
};

struct Selection {
    size_t             cursor_nr;
    struct PeviCursor *cursors;
};


struct UI {
    struct {
        Vector3 size;
        struct {
            Color body;
            Color wire;
        } color;
    } cursor;
};
// cursor_buffer
// owner -> file
// cell -> struct Cursor

struct File {
    char              *filename;
    struct linked_ring buffer;
    struct lr_cell    *cells;
};

struct Pevi {
    enum pevi_mode mode;
    bool           is_fps_visible;
    Camera_props_t cam;

    char command[COMMAND_BUFFER_SIZE];
    char key_buffer[COMMAND_BUFFER_SIZE];

    struct linked_ring cmd_buffer;
    struct lr_cell     cmd_cells[COMMAND_BUFFER_SIZE];

    struct PeviCursor cursor;
    struct Selection  selection;

    size_t       plane_nr;
    struct Plane planes[PLANE_BUFFER_SIZE];

    struct UI ui;
};


extern struct Pevi state;


void render_start(eer_t *win);
void render_end(eer_t *win);
