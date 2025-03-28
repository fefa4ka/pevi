#include "color_utils.h"
#include <stdlib.h>
#include <math.h>
#include "logger.h"

// Default settings for pastel colors
const ColorSettings_t PASTEL_COLOR_SETTINGS = {
    .saturation_min = 0.3f,
    .saturation_max = 0.5f,
    .value_min = 0.8f,
    .value_max = 1.0f,
    .alpha = 1.0f
};

// Generate a random color based on settings
Color color_random(const ColorSettings_t *settings) {
    if (!settings) {
        LOG_WARNING("Null settings in color_random, using default values");
        return color_random_pastel();
    }
    
    // Generate random hue (0-360)
    float hue = (float)(rand() % 360);
    
    // Generate random saturation within range
    float saturation_range = settings->saturation_max - settings->saturation_min;
    float saturation = settings->saturation_min + ((float)rand() / RAND_MAX) * saturation_range;
    
    // Generate random value/brightness within range
    float value_range = settings->value_max - settings->value_min;
    float value = settings->value_min + ((float)rand() / RAND_MAX) * value_range;
    
    // Convert HSV to RGB
    return color_hsv_to_rgb(hue, saturation, value, settings->alpha);
}

// Generate a random pastel color
Color color_random_pastel(void) {
    return color_random(&PASTEL_COLOR_SETTINGS);
}

// Convert HSV to RGB color
Color color_hsv_to_rgb(float h, float s, float v, float a) {
    Color rgb;
    
    // Make sure h is in the range 0-360
    h = fmodf(h, 360.0f);
    if (h < 0) h += 360.0f;
    
    // Clamp s and v to 0-1
    s = s < 0.0f ? 0.0f : (s > 1.0f ? 1.0f : s);
    v = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
    
    // Convert HSV to RGB
    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    
    float r, g, b;
    
    if (h < 60.0f) {
        r = c; g = x; b = 0;
    } else if (h < 120.0f) {
        r = x; g = c; b = 0;
    } else if (h < 180.0f) {
        r = 0; g = c; b = x;
    } else if (h < 240.0f) {
        r = 0; g = x; b = c;
    } else if (h < 300.0f) {
        r = x; g = 0; b = c;
    } else {
        r = c; g = 0; b = x;
    }
    
    // Convert to 0-255 range and add m
    rgb.r = (unsigned char)((r + m) * 255.0f);
    rgb.g = (unsigned char)((g + m) * 255.0f);
    rgb.b = (unsigned char)((b + m) * 255.0f);
    rgb.a = (unsigned char)(a * 255.0f);
    
    return rgb;
}
