
#include "pevi.h"

struct Pevi state = {.ui = {.cursor.size = (Vector3){0.6, 0.15, 0.6}}};

void shell();

struct lr_cell     cells[BUFFER_SIZE];
struct linked_ring file_buffer;

Clock(clk, &hw(timer), TIMESTAMP);
Camera_new(cam);
Window(win, _({"pevi", WINDOW_WIDTH, WINDOW_HEIGHT, &cam.state.camera,
               .on = {.before = render_start, .after = render_end}}));
Cursor_new(cur);
Text_new(txt);


// Shortcuts
Menu_command_t shortcuts[] = {{":", enable_command_mode},
                              {"i", enable_edit_mode},
                              {"e", enable_execute_mode},
                              {"m", enable_drag_mode},
                              {0}};
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
                             {"e", plane_open, state.command},
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


#define lr_last_cell(lr) ((lr)->cells + (lr)->size - 1)

void on_dot_hover(eer_t *symbol)
{

    eer_self(Symbol, symbol);

    if (state.mode == PEVI_MODE_FREE) {
        state.cursor.plane = (struct Plane *)self->props.owner;
        state.cursor.cell  = (struct lr_cell *)self->props.owner;
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
    state.cam.is_movable = false;


    state.cam.projection = CAMERA_ORTHOGRAPHIC;
    if (state.cursor.cell) {
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

void run_shell(char *command)
{
    FILE *fp;
    char  path[1035];

    /* Open the command for reading. */
    fp = popen(command, "r");
    if (fp == NULL) {

        lr_put(&file_buffer, (char)'\n', lr_owner(state.cursor.plane));

        lr_put_string(&file_buffer, "Failed to run command\n",
                      lr_owner(state.cursor.plane));
    }

    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path), fp) != NULL) {
        lr_put_string(&file_buffer, path, lr_owner(state.cursor.plane));
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
                lr_put(&file_buffer, (char)buffer[i],
                       lr_owner(state.cursor.plane));
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
                lr_put(&file_buffer, (char)buffer[i],
                       lr_owner(state.cursor.plane));
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
                lr_put(&file_buffer, (char)buffer[i],
                       lr_owner(state.cursor.plane));
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
    if (state.cursor.cell) {
        char command[COMMAND_BUFFER_SIZE];

        lr_read_string(&file_buffer, command, lr_owner(state.cursor.plane));
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
            lr_put(&file_buffer, (char)key, lr_owner(state.cursor.plane));
        }

        key = GetCharPressed(); // Check next character in the queue
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        int data;
        lr_pop(&file_buffer, &data, lr_owner(state.cursor.plane));
    } else if (IsKeyPressed(KEY_ESCAPE)) {
        state.mode                  = PEVI_MODE_FREE;
        state.cursor.plane          = 0;
        state.cam.is_movable        = true;
        state.cam.projection = CAMERA_PERSPECTIVE;


    } else if (IsKeyPressed(KEY_ENTER)) {
        // handle newline
        int len = TextLength(text);
        if (len < sizeof(text) - 1) {
            char command[COMMAND_BUFFER_SIZE];
            lr_read_string(&file_buffer, command, lr_owner(state.cursor.plane));
            lr_put(&file_buffer, (char)'\n', lr_owner(state.cursor.plane));
            //    run_shell(command);
        }
    } else if (IsKeyPressed(KEY_TAB)) {
        // handle newline
        int len = TextLength(text);
        if (len < sizeof(text) - 1) {
            lr_put(&file_buffer, (char)'\t', lr_owner(state.cursor.plane));
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
        lr_put_string(&file_buffer, text, lr_owner(state.cursor.plane));
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

    for (owner_cell = lr_last_cell(&file_buffer);
         owner_cell >= file_buffer.owners; owner_cell--) {

        lr_read_string(&file_buffer, text, lr_owner(owner_cell->data));
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
    lr_result_t result = lr_init(&file_buffer, BUFFER_SIZE, cells);
    result = lr_init(&state.cmd_buffer, COMMAND_BUFFER_SIZE, state.cmd_cells);

    Font fnt = GetFontDefault();

    state.cam = (Camera_props_t){(Vector3){55.0f, 0.0f, 1.0f},
                                 (Vector3){0.0f, 0.0f, 0.0f},
                                 (Vector3){0.0f, 1.0f, 0.0f},
                                 45.0f,
                                 CAMERA_PERSPECTIVE,
                                 CAMERA_FREE,
                                 true};


    ignite(clk, win);
    apply(Camera, cam, _(state.cam));


    if (state.mode == PEVI_MODE_DRAG) {
        struct Plane pln;
        pln                        = camera_plane(&cam.state.camera);
        state.cursor.plane->pos    = pln.pos;
        state.cursor.plane->angles = pln.angles;
    }
    state.cursor.cell = false;


    ClearBackground(RAYWHITE);

    react(Cursor, cur,
          _({
              .is_visible = state.mode == PEVI_MODE_FREE,
              .pos        = cam.state.camera.target,
              .size       = state.ui.cursor.size,
          }));

    // Foreach opened buffers (e.g. files, terminals);

    if (file_buffer.owners != NULL) {
        struct lr_cell *owner_cell;

        for (owner_cell = lr_last_cell(&file_buffer);
             owner_cell >= file_buffer.owners; owner_cell--) {



            lr_read_string(&file_buffer, text, lr_owner(owner_cell->data));

            struct Plane pln;
            Color        tint;
            pln  = *(struct Plane *)owner_cell->data;
            tint = pln.tint;

            if (state.mode == PEVI_MODE_EDIT
                && (struct Plane *)state.cursor.plane
                       == (struct Plane *)owner_cell->data) {
                pln      = camera_plane(&cam.state.camera);
                pln.tint = tint;
            }



            react(Text, txt,
                  _({.font         = GetFontDefault(),
                     .spacing      = 0.6f,
                     .tint         = BLACK,
                     .content      = text,
                     .owner        = owner_cell->data,
                     .font_size    = 12.0f,
		     .angles = pln.angles,
                     .pos = pln.pos,
                     .camera       = &cam.state.camera,
                     .bg_color     = pln.tint,
                     .on           = {.hover = on_dot_hover}}));
 // react(Text, txt,
 //                 _({.font         = GetFontDefault(),
 //                    .spacing      = 0.6f,
 //                    .tint         = BLACK,
 //                    .content      = text,
 //                    .owner        = owner_cell->data,
 //                    .font_size    = 12.0f,
 //       	     .angles = {0,0},
 //                    .pos = pln.pos,
 //                    .camera       = &cam.state.camera,
 //                    .bg_color     = pln.tint,
 //                    .on           = {.hover = on_dot_hover}}));

            if (state.mode == PEVI_MODE_EDIT
                && (size_t)state.cursor.plane == (size_t)owner_cell->data) {
                txt.state.pos.z += 1.f;
                Vector3 cursor_pos = txt.state.pos;
                cursor_pos.x += 0.5f;
                DrawCube(cursor_pos, 0.6f, 0.15f, 0.15f, PURPLE);
                DrawCubeWires(cursor_pos, 0.6f, 0.15f, 0.15f, DARKPURPLE);
            }

        }
    }


    halt(0);

    CloseWindow();
}

