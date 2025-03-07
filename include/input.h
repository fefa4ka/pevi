#pragma once
#include "camera.h"
#include "core.h"

// Input source types
typedef enum {
  INPUT_SOURCE_NONE,
  INPUT_SOURCE_SYMBOL,
  INPUT_SOURCE_TEXT,
  INPUT_SOURCE_PHANTOM,
  INPUT_SOURCE_SPACE,
  INPUT_SOURCE_KEYBOARD,
  INPUT_SOURCE_COMMAND
} InputSource_t;

// Mouse interaction types
typedef enum {
  INPUT_MOUSE_NONE,
  INPUT_MOUSE_HOVER,
  INPUT_MOUSE_CLICK,
  INPUT_MOUSE_DRAG,
  INPUT_MOUSE_RELEASE
} InputMouse_t;

// Keyboard interaction types
typedef enum {
  INPUT_KEY_NONE,
  INPUT_KEY_CHAR,
  INPUT_KEY_SPECIAL,
  INPUT_KEY_MODIFIER
} InputKey_t;

// Input event structure
typedef struct {
  // Source information
  InputSource_t source_type;
  void *source;
  
  // Keyboard state
  InputKey_t key_type;
  int key_code;
  bool shift_down;
  bool ctrl_down;
  bool alt_down;
  
  // Mouse state
  InputMouse_t mouse;
  Vector2 mouse_position;
  Vector2 mouse_delta;
  float mouse_wheel;
  
  // Callbacks for event handling
  struct {
    void (*hover)(void *source);
    void (*click)(void *source);
    void (*drag)(void *source);
    void (*release)(void *source);
    void (*key_press)(void *source, int key);
  } on;
} InputEvent_t;

// Input handler structure
typedef struct {
  // Current input state
  InputEvent_t current_event;
  
  // Previous input state for tracking changes
  InputEvent_t previous_event;
  
  // Mode-specific handlers
  void (*mode_handlers[5])(Pevi_t *pevi, InputEvent_t *event);
  
  // Cached key state
  int cached_key;
} InputHandler_t;

// Main input processing functions
void input_init(InputHandler_t *handler);
void input_process(Pevi_t *pevi, Camera_t *camera, InputHandler_t *handler);

// Mode-specific input handlers
void input_handle_free_mode(Pevi_t *pevi, InputEvent_t *event);
void input_handle_edit_mode(Pevi_t *pevi, InputEvent_t *event);
void input_handle_command_mode(Pevi_t *pevi, InputEvent_t *event);

// Helper functions
bool input_is_key_pressed(int key);
int input_get_char_pressed(void);
bool input_check_mode_switch(Pevi_t *pevi, InputEvent_t *event, Camera_t *camera);
