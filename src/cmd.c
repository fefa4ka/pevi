#include "pevi.h"

void enable_command_mode(void *arg)
{
	state.mode = PEVI_MODE_COMMAND;
	state.cam.is_movable = false;
}

void set_fovy(void *fovy_str)
{
    int fovy;
    sscanf(fovy_str, "%*s %d", &fovy);
    state.cam.fovy = fovy;
}

void print_fps(void *arg) { state.is_fps_visible = !state.is_fps_visible; }

void buffer_save(void *command)
{
    FILE           *file;
    char           *filename[COMMAND_BUFFER_SIZE];
    char text[BUFFER_SIZE];
    struct lr_cell *owner_cell;
    sscanf(command, "%*s %s", &filename);

    file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return;
    }

    for (owner_cell = lr_last_cell(state.file_buffer);
         owner_cell >= state.file_buffer->owners; owner_cell--) {

        lr_read_string(state.file_buffer, text, lr_owner(owner_cell->data));
        fprintf(file, "%s\n", text);
    }

    fclose(file);
}

void plane_open(void *command)
{
    FILE           *file;
    char           *filename[COMMAND_BUFFER_SIZE];
    char text[BUFFER_SIZE];
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
        lr_put_string(state.file_buffer, text, lr_owner(state.cursor.plane));
    }

    fclose(file);
}


void run_shell(char *command)
{
    FILE *fp;
    char  path[1035];

    /* Open the command for reading. */
    fp = popen(command, "r");
    if (fp == NULL) {

        lr_put(state.file_buffer, (char)'\n', lr_owner(state.cursor.plane));

        lr_put_string(state.file_buffer, "Failed to run command\n",
                      lr_owner(state.cursor.plane));
    }

    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path), fp) != NULL) {
        lr_put_string(state.file_buffer, path, lr_owner(state.cursor.plane));
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
                lr_put(state.file_buffer, (char)buffer[i],
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
         //       lr_put(state.file_buffer, (char)buffer[i],
         //              lr_owner(state.cursor.plane));
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
              //  lr_put(state.file_buffer, (char)buffer[i],
              //         lr_owner(state.cursor.plane));
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
void on_command(eer_t *menu)
{
    *state.command = 0;

    state.mode           = PEVI_MODE_FREE;
    state.cam.is_movable = true;
}

void on_command_not_found(eer_t *menu)
{
    *state.command = 0;

    state.mode           = PEVI_MODE_FREE;
    state.cam.is_movable = true;
}

void           buffer_dump(void *command) { lr_dump(&state.cmd_buffer); }

void quit(void *command)
{
    CloseWindow();
    exit(0);
}


