#include "core.h"
#include <stdio.h>

bool core_init(void) {
    bool success = true;
    
    printf("Initializing core components...\n");
    
    // Check if assets directory exists
    if (!DirectoryExists("assets")) {
        ERROR_SET(ERROR_FILE_NOT_FOUND, ERROR_WARNING, "Assets directory not found!");
        success = false;
    }
    
    // Check if fonts directory exists
    if (!DirectoryExists("assets/fonts")) {
        ERROR_SET(ERROR_FILE_NOT_FOUND, ERROR_WARNING, "Fonts directory not found!");
        success = false;
    }
    
    // Check if shaders directory exists
    if (!DirectoryExists("assets/shaders")) {
        ERROR_SET(ERROR_FILE_NOT_FOUND, ERROR_WARNING, "Shaders directory not found!");
        success = false;
    }
    
    // Check if font files exist
    if (!FileExists("assets/fonts/FiraCode-Regular.ttf")) {
        ERROR_SET(ERROR_FILE_NOT_FOUND, ERROR_WARNING, "Font file not found: assets/fonts/FiraCode-Regular.ttf");
        success = false;
    }
    
    if (!FileExists("assets/shaders/sdf.fs")) {
        ERROR_SET(ERROR_FILE_NOT_FOUND, ERROR_WARNING, "Shader file not found: assets/shaders/sdf.fs");
        success = false;
    }
    
    return success;
}
