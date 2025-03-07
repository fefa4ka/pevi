#pragma once
#include "symbol.h"
#include "camera.h"
#include "input.h"

bool text_draw(FontSettings_t *settings, char *content, Plane *plane, Camera_t *camera, InputEvent_t *event);
bool text_draw_on_plane(FontSettings_t *settings, char *content, Plane *plane, Camera_t *camera, InputEvent_t *event);
int text_codepoint_next(const char **text_ptr);
Vector4 text_measure(Font font, const char *text, float fontSize,
                            float fontSpacing, float lineSpacing);
