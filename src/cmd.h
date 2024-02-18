#pragma once
#include <eer.h>
#include <lr.h>
#include <stdbool.h>

extern bool command_mode;

void enable_edit_mode(void *arg);
void enable_execute_mode(void *arg);
void enable_drag_mode(void *arg);
void enable_command_mode(void *arg);

void set_fovy(void *fovy);
void print_fps(void *arg);

struct Buffer *buffer_init(size_t size);
void buffer_save(void *command);
void buffer_dump(void *command);

void file_open(void *command); 
void plane_open(void *command);
void plane_clean(void *command);


void run_shell(char *command);

void on_command(eer_t *menu);
void on_command_not_found(eer_t *menu);

void on_shortcut(eer_t *menu);
void on_shortcut_not_found(eer_t *menu);

void quit(void *command);
