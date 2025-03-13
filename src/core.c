#include "core.h"
#include "error.h"
#include "logger.h"

bool core_init(void) {
    bool success = true;
    
    LOG_INFO("Initializing core components...");
    
    // Check if assets directory exists
    if (!DirectoryExists("assets")) {
        LOG_WARNING("Assets directory not found!");
        ERROR_SET(ERROR_FILE_NOT_FOUND, ERROR_WARNING, "Assets directory not found!");
        success = false;
    } else {
        LOG_DEBUG("Assets directory found");
    }
    
    // Check if fonts directory exists
    if (!DirectoryExists("assets/fonts")) {
        LOG_WARNING("Fonts directory not found!");
        ERROR_SET(ERROR_FILE_NOT_FOUND, ERROR_WARNING, "Fonts directory not found!");
        success = false;
    } else {
        LOG_DEBUG("Fonts directory found");
    }
    
    // Check if shaders directory exists
    if (!DirectoryExists("assets/shaders")) {
        LOG_WARNING("Shaders directory not found!");
        ERROR_SET(ERROR_FILE_NOT_FOUND, ERROR_WARNING, "Shaders directory not found!");
        success = false;
    } else {
        LOG_DEBUG("Shaders directory found");
    }
    
    // Check if font files exist
    if (!FileExists("assets/fonts/FiraCode-Regular.ttf")) {
        LOG_WARNING("Font file not found: assets/fonts/FiraCode-Regular.ttf");
        ERROR_SET(ERROR_FILE_NOT_FOUND, ERROR_WARNING, "Font file not found: assets/fonts/FiraCode-Regular.ttf");
        success = false;
    } else {
        LOG_DEBUG("Font file found: assets/fonts/FiraCode-Regular.ttf");
    }
    
    if (!FileExists("assets/shaders/sdf.fs")) {
        LOG_WARNING("Shader file not found: assets/shaders/sdf.fs");
        ERROR_SET(ERROR_FILE_NOT_FOUND, ERROR_WARNING, "Shader file not found: assets/shaders/sdf.fs");
        success = false;
    } else {
        LOG_DEBUG("Shader file found: assets/shaders/sdf.fs");
    }
    
    if (success) {
        LOG_INFO("Core components initialized successfully");
    } else {
        LOG_WARNING("Core components initialized with warnings");
    }
    
    return success;
}
