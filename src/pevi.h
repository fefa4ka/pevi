#pragma once

#include <Camera.h>
#include <Clock.h>
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
#include "pemath.h"
#include "cmd.h"
#include "input.h"

enum pevi_mode { PEVI_MODE_FREE, PEVI_MODE_EDIT, PEVI_MODE_COMMAND };

struct Plane {
	Vector3 pos;
	Vector2 angles;
	Color tint;
};

struct Cursor {
	struct Plane *plane;			    
	struct linked_ring *buffer; // Text buffer
	struct lr_cell *cell;  // selected cell
};

struct Selection {
	size_t cursor_nr;
	struct Cursor *cursors;
};

// cursor_buffer
// owner -> file 
// cell -> struct Cursor

struct Pevi {
    enum pevi_mode mode;
    bool is_fps_visible;
    Camera_props_t cam;

    char command[COMMAND_BUFFER_SIZE];
    char key_buffer[COMMAND_BUFFER_SIZE];

    struct linked_ring cmd_buffer;
    struct lr_cell  cmd_cells[COMMAND_BUFFER_SIZE];

    struct Cursor cursor;
    struct Selection selection;

    struct Plane planes[PLANE_BUFFER_SIZE];
};



extern struct Pevi state;


void render_start(eer_t *win);
void render_end(eer_t *win);
