#pragma once
#include "core.h"

// Color generation settings
typedef struct {
    float saturation_min;  // Minimum saturation (0.0-1.0)
    float saturation_max;  // Maximum saturation (0.0-1.0)
    float value_min;       // Minimum value/brightness (0.0-1.0)
    float value_max;       // Maximum value/brightness (0.0-1.0)
    float alpha;           // Alpha value (0.0-1.0)
} ColorSettings_t;

// Default settings for pastel colors
extern const ColorSettings_t PASTEL_COLOR_SETTINGS;

// Generate a random color based on settings
Color color_random(const ColorSettings_t *settings);

// Generate a random pastel color
Color color_random_pastel(void);

// Convert HSV to RGB color
Color color_hsv_to_rgb(float h, float s, float v, float a);
