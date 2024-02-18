
#include "pevi.h"

struct Pevi state = {.ui = {.cursor.size = (Vector3){0.6, 0.15, 0.6}}};

void shell();

// struct lr_cell     cells[BUFFER_SIZE];
// struct linked_ring file_buffer;

Clock(clk, &hw(timer), TIMESTAMP);
Camera_new(cam);
Window(win, _({"pevi", WINDOW_WIDTH, WINDOW_HEIGHT, &cam.state.camera,
               .on = {.before = render_start, .after = render_end}}));
Cursor_new(cur);
Text_new(txt);
Terminal_new(trm);


// Shortcuts
Menu_command_t shortcuts[]
    = {{":", enable_command_mode}, {"i", enable_edit_mode},
       {"e", enable_execute_mode}, {"m", enable_drag_mode},
       {"x", plane_clean},         {0}};

Menu(skm, _({.menu    = shortcuts,
             .command = state.key_buffer,

             .on = {
                 .command   = on_shortcut,
                 .not_found = on_shortcut_not_found,
             }}));

// Commands
Menu_command_t commands[] = {{"fps", print_fps},
                             {"fovy", set_fovy, state.command},
                             {"w", buffer_save, state.command},
                             {"o", plane_open, state.command},
                             {"q", quit, state.command},
                             {"dump", buffer_dump},
                             {0}};
Menu(tty, _({.menu    = commands,
             .command = state.command,

             .on = {
                 .command   = on_command,
                 .not_found = on_command_not_found,
             }}));


char text[BUFFER_SIZE] = {0}; //{'p', 'e', 'v', 'i', '\0'};

void on_dot_hover(eer_t *symbol)
{
    // WARNING: Callback from Text and Symbol, props->parent, owner should be
    // aligned
    eer_self(Symbol, symbol);
    struct Buffer *buffer;
    buffer = (struct Buffer *)self->props.parent;

    if (state.mode == PEVI_MODE_FREE) {
        state.file_buffer     = &buffer->lr;
        state.cursor.buffer   = buffer;
        state.cursor.plane    = (struct Plane *)self->props.owner;
        state.cursor.cell     = (struct lr_cell *)self->props.parent_arg;
        state.cursor.index    = (size_t)self->props.parent_arg + 1;
        state.cursor.position = self->state.pos;
        state.cursor.size     = self->state.size;

        state.cursor.is_hovered = true;
    }
}

void on_cursor_symbol_render(eer_t *symbol)
{

    // WARNING: Callback from Text and Symbol, props->parent, owner should be
    // aligned
    eer_self(Symbol, symbol);

    struct Buffer *buffer;
    buffer = (struct Buffer *)self->props.parent;

    if (state.mode == PEVI_MODE_EDIT) {
        state.file_buffer     = &buffer->lr;
        state.cursor.index    = (size_t)self->props.parent_arg;
        state.cursor.position = self->state.pos;
        state.cursor.size     = self->state.size;
    }
}


Serial(input,
       _({.handler = &eer_keyboard,
          .buffer  = &state.cmd_buffer,
          .on      = {
                   .receive_block = read_command, /* Read and execute command */
                   .receive       = read_symbol   /* Echo input */
          }}));

void render_start(eer_t *win)
{
    // BeginMode3D(cam.state.camera); // FIXME: Blinking appears
}
bool      selection_mode   = false;
Vector2   selection_start  = {0};
Vector2   selection_finish = {0};
Rectangle rec              = {0, 0, 0, 0};
void      render_end(eer_t *win)
{
    EndMode3D();

    if (state.is_fps_visible) {
        DrawFPS(10, 10);
    }

    if (state.mode == PEVI_MODE_COMMAND) {
        DrawText(state.key_buffer, 10, GetScreenHeight() - 70, 30, WHITE);
    }

    if (selection_mode) {
        Vector2 start;
        Vector2 finish;
        if (selection_start.x > selection_finish.x) {
            start.x  = selection_finish.x;
            finish.x = selection_start.x;
        } else {
            start.x  = selection_start.x;
            finish.x = selection_finish.x;
        }
        if (selection_start.y > selection_finish.y) {
            start.y  = selection_finish.y;
            finish.y = selection_start.y;
        } else {
            start.y  = selection_start.y;
            finish.y = selection_finish.y;
        }
        rec.x      = start.x;
        rec.y      = start.y;
        rec.width  = (finish.x - start.x);
        rec.height = (finish.y - start.y);

        DrawRectangleRec(rec, Fade(GRAY, 0.5f));
    }

    if (state.cursor.is_hovered) {
        char *path;
        if (state.cursor.buffer) {
            path = state.cursor.buffer->path;

            if (path) {

                DrawText(path, 10, GetScreenHeight() - 70, 30, WHITE);
            }
        }
    }
    shell();
}
void on_shortcut(eer_t *menu)
{
    lr_data_t data;

    while (Serial_read(&input, &data) == OK && data) {
    }
    int r
        = lr_read_string(&state.cmd_buffer, state.key_buffer,
                         lr_owner(eer_prop(Serial, &input, handler)->receive));
    if (r != OK) {
        state.key_buffer[0] = '\0';
    }
}

void on_shortcut_not_found(eer_t *menu)
{
    lr_data_t data;

    while (Serial_read(&input, &data) == OK && data) {
    }
    int r
        = lr_read_string(&state.cmd_buffer, state.key_buffer,
                         lr_owner(eer_prop(Serial, &input, handler)->receive));
    if (r != OK) {
        state.key_buffer[0] = '\0';
    }
}

void read_symbol(eer_t *uart_ptr)
{
    // FIXME: hack
    if (state.mode == PEVI_MODE_COMMAND
        && eer_state(Serial, &input, sending) == '\b') {
        int data;
        lr_pop(&state.cmd_buffer, &data,
               lr_owner(eer_prop(Serial, &input, handler)->receive));
        lr_pop(&state.cmd_buffer, &data,
               lr_owner(eer_prop(Serial, &input, handler)->receive));
    }

    if (state.mode != PEVI_MODE_EDIT)
        lr_read_string(&state.cmd_buffer, state.key_buffer,
                       lr_owner(eer_prop(Serial, &input, handler)->receive));
}


void read_command(eer_t *uart)
{
    char     *command_symbol = state.command;
    lr_data_t data;
    while (Serial_read(&input, &data) == OK && data) {
        *command_symbol++ = (uint8_t)data;
    }
    *--command_symbol = 0;
}

struct Plane camera_plane(Camera *camera)
{
    float saturation = 0.1f; // High saturation for vivid colors
    float value      = 0.8f; // High value for bright colors
                             //
    Vector2 angles = CalculateBillboardAngles(camera->target, camera->position,
                                              camera->up);

    return (struct Plane){.pos    = camera->target,
                          .angles = angles,
                          .tint   = GenerateRandomColor(saturation, value)};
}

void enable_edit_mode(void *args)
{
    state.mode           = PEVI_MODE_EDIT;
    state.cam.is_enabled = false;

    if (!state.file_buffer) {
        struct Buffer *buffer;
        buffer              = buffer_init(BUFFER_SIZE);
        state.cursor.buffer = buffer;
        state.file_buffer   = &buffer->lr;
    }

    state.cam.projection = CAMERA_ORTHOGRAPHIC;
    if (state.cursor.is_hovered) {
        return;
    }

    state.planes[state.plane_nr] = camera_plane(&cam.state.camera);
    state.cursor.plane           = &state.planes[state.plane_nr];
    state.plane_nr += 1;
}

size_t current_drag_cursor_index = 0;
void   enable_drag_mode(void *args)
{
    if (state.mode == PEVI_MODE_DRAG) {
        state.mode = PEVI_MODE_FREE;
    } else {
        state.mode = PEVI_MODE_DRAG;
    }
}


void enable_execute_mode(void *args)
{

    lr_data_t      data;
    struct Buffer *buffer;
    buffer = state.cursor.buffer;
    if (buffer->type == PEVI_BUF_FILE) {
        char command[COMMAND_BUFFER_SIZE];

        struct linked_ring *lr = state.file_buffer;
        lr_read_string(lr, command, lr_owner(state.cursor.plane));
        while (lr_get(lr, &data, lr_owner(state.cursor.plane)) == OK)
            ;
        run_shell(command);
    } else if (buffer->type == PEVI_BUF_TERMINAL) {
        while (lr_get(&buffer->lr, &data, lr_owner(state.cursor.plane)) == OK)
            ;
        pclose(buffer->fp);
        buffer->fp = popen(buffer->path, "r");
        int fd     = fileno(buffer->fp);
        if (buffer->fp == NULL) {
            exit(0);
        }
    }
}

/* Application */
void edit_mode_process()
{
    int key;
    int r = Serial_read(&input, &key);

    state.cursor.is_hovered         = true;
    struct linked_ring *file_buffer = state.file_buffer;
    if (r) {
        key = 0;
    }

    while (key > 0) {
        // NOTE: Only allow keys in range [32..125]
        if ((key >= 32) && (key <= 125)) {
            if (state.cursor.index != -1) {
                lr_insert(file_buffer, (char)key, lr_owner(state.cursor.plane),
                          state.cursor.index++);
                //            lr_insert_next(file_buffer, (char)key,
                //            state.cursor.cell);
            } else {

                lr_put(file_buffer, (char)key, lr_owner(state.cursor.plane));
            }
        }

        key = GetCharPressed(); // Check next character in the queue
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        int data;

        if (state.cursor.index != -1) {
            lr_pull(file_buffer, &data, lr_owner(state.cursor.plane),
                    --state.cursor.index );
        } else {
            lr_pop(file_buffer, &data, lr_owner(state.cursor.plane));
        }
    } else if (IsKeyPressed(KEY_ESCAPE)) {
        state.mode           = PEVI_MODE_FREE;
        state.cursor.plane   = 0;
        state.cam.is_enabled = true;
        state.cam.projection = CAMERA_PERSPECTIVE;


    } else if (IsKeyPressed(KEY_ENTER)) {
        // handle newline
        int len = TextLength(text);
        if (len < sizeof(text) - 1) {
            char command[COMMAND_BUFFER_SIZE];
            lr_read_string(file_buffer, command, lr_owner(state.cursor.plane));

            lr_put(file_buffer, (char)'\n', lr_owner(state.cursor.plane));
            //    run_shell(command);
        }
    } else if (IsKeyPressed(KEY_TAB)) {
        // handle newline
        int len = TextLength(text);
        if (len < sizeof(text) - 1) {
            lr_put(file_buffer, (char)'\t', lr_owner(state.cursor.plane));
        }
    } else if (IsKeyPressed(KEY_LEFT)) {
        state.cursor.index -= 1;
    } else if (IsKeyPressed(KEY_RIGHT)) {
        state.cursor.index += 1;
    } else if (IsKeyPressed(KEY_UP)) {
        int index = state.cursor.index;
        while (index > 0) {
            if (text[index] == '\n')
                break;

            index--;
        }
        state.cursor.index = --index;
    } else if (IsKeyPressed(KEY_DOWN)) {
        int           index = state.cursor.index;
        unsigned long len   = strlen(text);
        while (len > index) {
            if (text[index] == '\n')
                break;

            index++;
        }
        state.cursor.index = ++index;
    }
}


void shell()
{
    ignite(input);
    if (state.mode == PEVI_MODE_COMMAND) {
        use(tty);
    } else if (state.mode == PEVI_MODE_EDIT) {
        edit_mode_process();
    } else {
        use(skm);
    }
}

int main(void)
{
    lr_result_t result;
    //= lr_init(&file_buffer, BUFFER_SIZE, cells);
    result = lr_init(&state.cmd_buffer, COMMAND_BUFFER_SIZE, state.cmd_cells);

    Font fnt = GetFontDefault();

    state.cam = (Camera_props_t){(Vector3){55.0f, 0.0f, 1.0f},
                                 (Vector3){0.0f, 0.0f, 0.0f},
                                 (Vector3){0.0f, 1.0f, 0.0f},
                                 45.0f,
                                 CAMERA_PERSPECTIVE,
                                 CAMERA_THIRD_PERSON,
                                 true};
    bool   is_drag_set;
    Camera drag_cam;

    //    state.file_buffer = &file_buffer;


    SetTargetFPS(30);
    // Main loop
    ignite(clk, win);

    if (IsMouseButtonDown(0)) {
        if (!selection_mode) {
            selection_start = GetMousePosition();
            selection_mode  = true;
        } else {
            selection_finish = GetMousePosition();
        }
    } else if (IsMouseButtonUp(0)) {
        selection_mode = false;
    }

    if (IsKeyDown(KEY_LEFT_SHIFT) /* || state.mode == PEVI_MODE_DRAG*/) {
        state.cam.is_mouse_enabled = true;
    } else {

        state.cam.is_mouse_enabled = false;
    }

    struct Plane cam_plane  = camera_plane(&cam.state.camera);
    Vector3      cursor_pos = {0};

    struct Plane pln;
    if (state.mode == PEVI_MODE_DRAG && !is_drag_set) {
        //       pln                        = camera_plane(&cam.state.camera);
        //       state.cursor.plane->pos    = pln.pos;
        //       state.cursor.plane->angles = pln.angles;
    }

    if (IsMouseButtonDown(1)) {

        Vector3 movement = {0};
        Vector3 rotation = {0};

        if (state.mode != PEVI_MODE_DRAG) {
            state.mode = PEVI_MODE_DRAG;

            printf("Click\n");
            if (!is_drag_set) {
                //SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
                drag_cam = cam.state.camera;

                printf("UPDATE\n");
                is_drag_set = true;
            }
	    if(GetMousePosition().x > GetScreenWidth() / 2)
		    movement.y = fabs(GetScreenWidth() / 2 - GetMousePosition().x);
	    else
		    movement.y = (GetScreenWidth() / 2 - GetMousePosition().x) * -1;

	    if(GetMousePosition().y > GetScreenHeight() / 2)
		    movement.z = fabs(GetScreenHeight() / 2 - GetMousePosition().y);
	    else
	    movement.z = (GetScreenHeight() / 2 - GetMousePosition().y) * -1;

	    movement.y*=2;
	    movement.z*=2;
        } else{

		movement.y       = GetMouseDelta().x;
		movement.z       = GetMouseDelta().y;
	}

        float   distance = Vector3Distance(drag_cam.position, drag_cam.target);
        float   coeff    = distance / 2700;
	movement.y = movement.y * coeff;
	movement.z = movement.z  * -1 * coeff;

        UpdateCameraPro(&drag_cam, movement, rotation, 0);

        pln                        = camera_plane(&drag_cam);
        state.cursor.plane->pos    = pln.pos;

	// Saving original angles
	if(!IsKeyDown(KEY_LEFT_CONTROL))
		state.cursor.plane->angles = pln.angles;
	
    } else {
        if (state.mode == PEVI_MODE_DRAG && is_drag_set == true) {
            state.mode  = PEVI_MODE_FREE;
            is_drag_set = false;
        }
    }

    apply(Camera, cam, _(state.cam));


    state.cursor.is_hovered = false;

    if (state.mode == PEVI_MODE_FREE)
        state.cursor.index = -1;


    ClearBackground(BLANK);


    // Foreach opened buffers (e.g. files, terminals);
    struct Buffer *buffer;
    buffer = state.buffer;
    if (buffer) {
        do {
            struct linked_ring *file_buffer = &buffer->lr;

            if (file_buffer->owners != NULL) {
                struct lr_cell *owner_cell;

                for (owner_cell = lr_last_cell(file_buffer);
                     owner_cell >= file_buffer->owners; owner_cell--) {


                    lr_read_string(file_buffer, text,
                                   lr_owner(owner_cell->data));

                    Color tint;
                    pln  = *(struct Plane *)owner_cell->data;
                    tint = pln.tint;

                    if (state.mode == PEVI_MODE_EDIT
                        && (struct Plane *)state.cursor.plane
                               == (struct Plane *)owner_cell->data) {
                        pln      = cam_plane;
                        pln.tint = tint;
                    }


                    if (buffer->type == PEVI_BUF_FILE) {

                        bool is_selected;
                        // Selection
                        Rectangle text_rec;
                        Matrix    transform;
                        transform
                            = MatrixTranslate(pln.pos.x, pln.pos.y, pln.pos.z);
                        transform = MatrixMultiply(MatrixRotateY(pln.angles.y),
                                                   transform);
                        transform = MatrixMultiply(MatrixRotateX(pln.angles.x),
                                                   transform);
                        transform = MatrixMultiply(
                            MatrixTranslate(txt.state.size.x, 0,
                                            txt.state.size.z),
                            transform);
                        Vector3 finalPosition = {0, 0, 0};
                        finalPosition
                            = Vector3Transform(finalPosition, transform);
                        Vector2 text_rec_pos
                            = GetWorldToScreen(pln.pos, cam.state.camera);
                        text_rec.x = text_rec_pos.x;
                        text_rec.y = text_rec_pos.y;
                        Vector2 text_rec_pos_right
                            = GetWorldToScreen(finalPosition, cam.state.camera);
                        text_rec.width  = text_rec_pos_right.x - text_rec_pos.x;
                        text_rec.height = text_rec_pos_right.y - text_rec_pos.y;

                        is_selected = false;
                        if (CheckCollisionRecs(rec, text_rec)) {
                            is_selected = true;
                        }
                        //


                        shoot(Text, txt,
                              _({.font        = win.state.font,
                                 .spacing     = 0.6f,
                                 .tint        = BLACK,
                                 .content     = text,
                                 .parent      = (void *)buffer,
                                 .owner       = owner_cell->data,
                                 .parent_arg  = (void *)state.cursor.index,
                                 .font_size   = 24.,
                                 .angles      = pln.angles,
                                 .pos         = pln.pos,
                                 .camera      = &cam.state.camera,
                                 .bg_color    = pln.tint,
                                 .shader      = win.state.shader,
                                 .is_selected = is_selected,
                                 .on          = {.hover  = on_dot_hover,
                                                 .cursor = on_cursor_symbol_render}}));
                    }


                    if (state.mode == PEVI_MODE_EDIT
                        && (size_t)state.cursor.plane
                               == (size_t)owner_cell->data) {
                        if (state.cursor.index != -1) {
                            cursor_pos = state.cursor.position;
                            cursor_pos.x += state.cursor.size.x + 0.2;
                            cursor_pos.z += state.cursor.size.z;
                        } else {
                            cursor_pos.x = txt.state.pos.x + 0.2;
                            cursor_pos.y = txt.state.pos.y;
                            cursor_pos.z = txt.state.size.z;
                        }
                    }
                }
            }

            if (buffer->type == PEVI_BUF_TERMINAL) {
                struct Plane  pln;
                struct Plane *pln_ptr;
                Color         tint;

                if (buffer->plane) {
                    pln_ptr = buffer->plane;
                } else {
                    state.planes[state.plane_nr] = cam_plane;
                    pln_ptr = &state.planes[state.plane_nr];
                    state.plane_nr += 1;
                    buffer->plane = pln_ptr;
                }
                pln = *pln_ptr;

                if (state.mode == PEVI_MODE_DRAG) {
                    struct Plane pln;
                    pln                     = camera_plane(&cam.state.camera);
                    state.cursor.plane->pos = pln.pos;
                    state.cursor.plane->angles = pln.angles;
                }
                shoot(Terminal, trm,
                      _({.fp        = buffer->fp,
                         .command   = buffer->path,
                         .buffer    = &buffer->lr,
                         .parent    = (void *)buffer,
                         .font      = win.state.font,
                         .spacing   = 0.6f,
                         .tint      = BLACK,
                         .owner     = lr_owner(pln_ptr),
                         .font_size = 24.,
                         .angles    = pln.angles,
                         .pos       = pln.pos,
                         .camera    = &cam.state.camera,
                         .bg_color  = pln.tint,
                         .shader    = win.state.shader,
                         .on        = {.hover = on_dot_hover}

                      }));
            }


            buffer = buffer->next;
        } while (state.buffer != buffer);
    }

    react(Cursor, cur,
          _({
              .is_visible   = true,
              .pos          = cursor_pos,
              .absolute_pos = cam.state.camera.target,
              .size         = state.ui.cursor.size,
              .angles
              = state.mode == PEVI_MODE_EDIT ? cam_plane.angles : (Vector2){0},
          }));

    halt(0);

    CloseWindow();
}

