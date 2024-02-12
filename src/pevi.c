
#include "pevi.h"

struct Pevi state;

void shell();

#define BUFFER_SIZE 1000
struct lr_cell     cells[BUFFER_SIZE];
struct linked_ring lr;

Clock(clk, &hw(timer), TIMESTAMP);
Camera_new(cam);

void render_start(eer_t *win)
{
    //	BeginMode3D(cam.state.camera); // WHY THIS DON'T WORK?
}

void render_end(eer_t *win);

Window(win, _({"pevi", WINDOW_WIDTH, WINDOW_HEIGHT, &cam.state.camera,
               .on = {.before = render_start, .after = render_end}}));
Text_new(txt);
Symbol_new(dot);

void quit(void *command)
{
    CloseWindow();
    exit(0);
}


void           buffer_dump(void *command) { lr_dump(&lr); }
Menu_command_t commands[] = {{"fps", print_fps},
                             {"fovy", set_fovy, state.command},
                             {"w", buffer_save, state.command},
                             {"e", plane_open, state.command},
                             {"q", quit, state.command},
                             {"dump", buffer_dump},
                             {0}};

Menu_command_t shortcuts[] = {{":", enable_command_mode},
                              {"i", enable_edit_mode},
                              {"e", enable_execute_mode},
                              {"m", enable_drag_mode},
                              {0}};



Menu(tty, _({.menu    = commands,
             .command = state.command,

             .on = {
                 .command   = on_command,
                 .not_found = on_command_not_found,
             }}));


Menu(skm, _({.menu    = shortcuts,
             .command = state.key_buffer,

             .on = {
                 .command   = on_shortcut,
                 .not_found = on_shortcut_not_found,
             }}));


char text[BUFFER_SIZE] = {0}; //{'p', 'e', 'v', 'i', '\0'};

// struct Cursor cursors[CURSOR_BUFFER_SIZE];

Vector3 cursor_position[BUFFER_SIZE];
Vector3 cursor_cam_position[BUFFER_SIZE];
Vector3 cursor_cam_up[BUFFER_SIZE];


bool     current_symbol;
size_t   current_symbol_owner;
size_t   current_symbol_index;
size_t   cursor_index         = 1;
size_t   current_cursor_index = 1;
Vector3 *current_cursor;
Color    cursor_color[BUFFER_SIZE];

#define lr_last_cell(lr) ((lr)->cells + (lr)->size - 1)

void on_dot_hover(eer_t *symbol)
{
    eer_self(Symbol, symbol);
    current_symbol       = true;
    current_symbol_owner = (size_t)self->props.owner;
    current_symbol_index = (size_t)self->props.content_index;
}


Serial(input,
       _({.handler = &eer_keyboard,
          .buffer  = &state.cmd_buffer,
          .on      = {
                   .receive_block = read_command, /* Read and execute command */
                   .receive       = read_symbol   /* Echo input */
          }}));

void render_end(eer_t *win)
{
    EndMode3D();
    if (state.is_fps_visible) {
        DrawFPS(10, 10);
    }

    if (state.mode == PEVI_MODE_COMMAND) {
        DrawText(state.key_buffer, 10, GetScreenHeight() - 20, 10, GRAY);
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
        && eer_state(Serial, &input, sending) == 0) {
        int data;
        lr_pop(&state.cmd_buffer, &data,
               lr_owner(eer_prop(Serial, &input, handler)->receive));
        lr_pop(&state.cmd_buffer, &data,
               lr_owner(eer_prop(Serial, &input, handler)->receive));
    }
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



void enable_edit_mode(void *args)
{
    state.mode           = PEVI_MODE_EDIT;
    state.cam.is_movable = false;

    if (current_symbol) {
        current_cursor_index = current_symbol_owner;
        Vector3 text_pos     = cursor_position[current_cursor_index];
        Vector3 cam_pos      = cursor_cam_position[current_cursor_index];
        Vector3 cam_up       = cursor_cam_up[current_cursor_index];

        Vector2 angles = CalculateBillboardAngles(text_pos, cam_pos, cam_up);

        cam.state.camera.position = cam_pos;
        cam.state.camera.up       = cam_up;
        cam.state.camera.target   = text_pos;

    } else {
        float saturation = 0.1f; // High saturation for vivid colors
        float value      = 0.8f; // High value for bright colors
                                 //
        Vector2 angles = CalculateBillboardAngles(cam.state.camera.target,
                                                  cam.state.camera.position,
                                                  cam.state.camera.up);

        struct Cursor cur = {.pos    = cam.state.camera.target,
                             .angles = angles,
                             .tint   = GenerateRandomColor(saturation, value)};

        cursor_position[cursor_index]     = cam.state.camera.target;
        cursor_cam_position[cursor_index] = cam.state.camera.position;
        cursor_cam_up[cursor_index]       = cam.state.camera.up;
        current_cursor                    = &cursor_position[cursor_index];
        current_cursor_index              = cursor_index;
        Color randomColor          = GenerateRandomColor(saturation, value);
        cursor_color[cursor_index] = randomColor;
        cursor_index += 1;
    }
}

bool   drag_mode                 = false;
size_t current_drag_cursor_index = 0;
void   enable_drag_mode(void *args)
{
    drag_mode                 = !drag_mode;
    current_drag_cursor_index = current_symbol_owner;
}

void run_shell(char *command)
{
    FILE *fp;
    char  path[1035];

    /* Open the command for reading. */
    fp = popen(command, "r");
    if (fp == NULL) {

        lr_put(&lr, (char)'\n', lr_owner(current_cursor_index));

        lr_put_string(&lr, "Failed to run command\n",
                      lr_owner(current_cursor_index));
    }

    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path), fp) != NULL) {
        lr_put_string(&lr, path, lr_owner(current_cursor_index));
    }

    /* close */
    pclose(fp);
    state.cam.is_movable = true;
    state.mode           = PEVI_MODE_FREE;
}

void run_shell_interactive(char *command)
{
    int   fd[2]; // File descriptors for pipe
    pid_t pid;
    char  buffer[BUFSIZ];

    // Create a pipe
    if (pipe(fd) == -1) {
        perror("pipe");
        return;
    }

    // Fork a child process
    pid = fork();
    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {   // Child process
        close(fd[0]); // Close reading end of pipe

        // Redirect stdout to the writing end of the pipe
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        // Execute the command
        execlp("sh", "sh", "-c", command, NULL);
        perror("execlp");
    } else {          // Parent process
        close(fd[1]); // Close writing end of pipe

        // Read from the reading end of the pipe
        ssize_t bytes_read;
        while ((bytes_read = read(fd[0], buffer, BUFSIZ)) > 0) {
            // Process the data, e.g., display it to the user
            write(STDOUT_FILENO, buffer, bytes_read);
            for (ssize_t i = 0; i < bytes_read; i++) {
                // Write each character individually
                lr_put(&lr, (char)buffer[i], lr_owner(current_cursor_index));
            }
        }

        close(fd[0]); // Close reading end of pipe
    }
}


void run_command_interactively(char *command)
{
    FILE *fp;
    char  buffer[BUFSIZ];

    // Open the command for reading
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen");
        return;
    }

    // Read from the command and write to stdout
    fd_set fds;
    int    fd = fileno(fp);
    while (1) {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        FD_SET(STDIN_FILENO, &fds);

        // Use select() to wait for data on either stdin or command output
        if (select(fd + 1, &fds, NULL, NULL, NULL) == -1) {
            perror("select");
            break;
        }

        // If data is available on stdin, read it and write to the command
        if (FD_ISSET(STDIN_FILENO, &fds)) {
            ssize_t bytes_read = read(STDIN_FILENO, buffer, BUFSIZ);
            if (bytes_read <= 0)
                break;

            for (ssize_t i = 0; i < bytes_read; i++) {
                // Write each character individually
                lr_put(&lr, (char)buffer[i], lr_owner(current_cursor_index));
            }
            // if (write(fd, buffer, bytes_read) == -1) {
            //     perror("write");
            //     break;
            // }
        }

        // If data is available on the command output, read it and write to
        // stdout
        if (FD_ISSET(fd, &fds)) {
            ssize_t bytes_read = read(fd, buffer, BUFSIZ);
            if (bytes_read <= 0)
                break;

            for (ssize_t i = 0; i < bytes_read; i++) {
                // Write each character individually
                lr_put(&lr, (char)buffer[i], lr_owner(current_cursor_index));
            }
            //   if (write(STDOUT_FILENO, buffer, bytes_read) == -1) {
            //       perror("write");
            //       break;
            //   }
        }
    }

    // Close the file stream
    pclose(fp);
}

void enable_execute_mode(void *args)
{
    if (current_symbol) {
        char command[COMMAND_BUFFER_SIZE];
        current_cursor_index = current_symbol_owner;

        lr_read_string(&lr, command, lr_owner(current_cursor_index));
        run_command_interactively(command);
    }
}

/* Application */
void edit_mode_process()
{
    int key;
    int r = Serial_read(&input, &key);
    if (r) {
        key = 0;
    }

    while (key > 0) {
        // NOTE: Only allow keys in range [32..125]
        if ((key >= 32) && (key <= 125)) {
            lr_put(&lr, (char)key, lr_owner(current_cursor_index));
        }

        key = GetCharPressed(); // Check next character in the queue
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        int data;
        lr_pop(&lr, &data, lr_owner(current_cursor_index));
    } else if (IsKeyPressed(KEY_ESCAPE)) {
        state.mode           = PEVI_MODE_FREE;
        state.cam.is_movable = true;
        current_symbol       = 0;
    } else if (IsKeyPressed(KEY_ENTER)) {
        // handle newline
        int len = TextLength(text);
        if (len < sizeof(text) - 1) {
            char command[COMMAND_BUFFER_SIZE];
            lr_read_string(&lr, command, lr_owner(current_cursor_index));
            lr_put(&lr, (char)'\n', lr_owner(current_cursor_index));
            //    run_shell(command);
        }
    } else if (IsKeyPressed(KEY_TAB)) {
        // handle newline
        int len = TextLength(text);
        if (len < sizeof(text) - 1) {
            lr_put(&lr, (char)'\t', lr_owner(current_cursor_index));
        }
    }
}


void plane_open(void *command)
{
    FILE           *file;
    char           *filename[COMMAND_BUFFER_SIZE];
    struct lr_cell *owner_cell;
    sscanf(command, "%*s %s", &filename);

    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return;
    }

    enable_edit_mode(0);
    /* Read the output a line at a time - output it. */
    while (fgets(text, sizeof(text), file) != NULL) {
        lr_put_string(&lr, text, lr_owner(current_cursor_index));
    }

    fclose(file);
}

void buffer_save(void *command)
{
    FILE           *file;
    char           *filename[COMMAND_BUFFER_SIZE];
    struct lr_cell *owner_cell;
    sscanf(command, "%*s %s", &filename);

    file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return;
    }

    for (owner_cell = lr_last_cell(&lr); owner_cell >= lr.owners;
         owner_cell--) {

        lr_read_string(&lr, text, lr_owner(owner_cell->data));
        fprintf(file, "%s\n", text);
    }

    fclose(file);
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
    lr_result_t result = lr_init(&lr, BUFFER_SIZE, cells);
    result = lr_init(&state.cmd_buffer, COMMAND_BUFFER_SIZE, state.cmd_cells);

    state.cam = (Camera_props_t){(Vector3){10.0f, 0.0f, 1.0f},
                                 (Vector3){0.0f, 0.0f, 0.0f},
                                 (Vector3){0.0f, 1.0f, 0.0f},
                                 45.0f,
                                 CAMERA_PERSPECTIVE,
                                 CAMERA_FREE,
                                 true};

    //    Font fnt = LoadFontEx("FiraCode-Regular.ttf", 96, 0, 0);
    Font fnt = GetFontDefault();

    ignite(clk, win);

    apply(Camera, cam, _(state.cam));

    if (drag_mode) {
        cursor_position[current_drag_cursor_index] = cam.state.camera.target;
        cursor_cam_position[current_drag_cursor_index]
            = cam.state.camera.position;
        cursor_cam_up[current_drag_cursor_index] = cam.state.camera.up;
        current_cursor = &cursor_position[current_drag_cursor_index];
    }

    current_symbol = false;

    ClearBackground(RAYWHITE);
    //    DrawGrid(100, 1.0f);
    if (state.mode == PEVI_MODE_FREE) {
        rlPushMatrix();
        Vector3 pos    = cam.state.camera.target;
        Vector2 angles = CalculateBillboardAngles(
            pos, cam.state.camera.position, cam.state.camera.up);

        //            rlTranslatef(pos.x, pos.y, pos.z);
        //
        //            rlRotatef(RAD2DEG * angles.x, 0, 1,
        //                      0); // Rotate around Y-axis (yaw)
        //            rlRotatef(RAD2DEG * angles.y, 1, 0,
        //                      0); // Rotate around X-axis (pitch)

        DrawCube(pos, 1.0f, 0.25f, 1.0f, PURPLE);
        DrawCubeWires(pos, 1.0f, 0.25f, 1.0f, DARKPURPLE);
        rlPopMatrix();
    }


    if (lr.owners != NULL) {
        struct lr_cell *owner_cell;

        for (owner_cell = lr_last_cell(&lr); owner_cell >= lr.owners;
             owner_cell--) {

            rlPushMatrix();

            lr_read_string(&lr, text, lr_owner(owner_cell->data));
            Vector3 text_poss = cursor_position[(size_t)owner_cell->data];
            Vector3 cam_pos   = cursor_cam_position[(size_t)owner_cell->data];
            Vector3 cam_up    = cursor_cam_up[(size_t)owner_cell->data];
            Color   bg_color  = cursor_color[(size_t)owner_cell->data];
            Vector3 text_pos  = {0};

            Vector2 angles
                = CalculateBillboardAngles(text_poss, cam_pos, cam_up);

            rlTranslatef(text_poss.x, text_poss.y, text_poss.z);

            rlRotatef(RAD2DEG * angles.x, 0, 1,
                      0); // Rotate around Y-axis (yaw)
            rlRotatef(RAD2DEG * angles.y, 1, 0,
                      0); // Rotate around X-axis (pitch)


            react(Text, txt,
                  _({.font         = GetFontDefault(),
                     .spacing      = 0.6f,
                     .tint         = BLACK,
                     .content      = text,
                     .owner        = owner_cell->data,
                     .font_size    = 12.0f,
                     .pos          = text_pos,
                     .absolute_pos = text_poss,
                     .camera       = &cam.state.camera,
                     .bg_color     = bg_color,
                     .on           = {.hover = on_dot_hover}}));

            if (state.mode == PEVI_MODE_EDIT
                && (size_t)current_cursor_index == (size_t)owner_cell->data) {
                txt.state.pos.z += 1.f;
                Vector3 cursor_pos = txt.state.pos;
                cursor_pos.x += 0.5f;
                DrawCube(cursor_pos, 0.8f, 0.25f, 0.25f, PURPLE);
                DrawCubeWires(cursor_pos, 0.8f, 0.25f, 0.25f, DARKPURPLE);
            }

            rlPopMatrix();
        }
    }


    halt(0);

    CloseWindow();
}

