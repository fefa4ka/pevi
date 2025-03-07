#include "core.h"
#include <stdio.h>

void core_init(void) {
    printf("Initializing core components...\n");
    
    // Check if assets directory exists
    if (!DirectoryExists("assets")) {
        printf("Warning: Assets directory not found!\n");
    }
    
    // Check if fonts directory exists
    if (!DirectoryExists("assets/fonts")) {
        printf("Warning: Fonts directory not found!\n");
    }
    
    // Check if font files exist
    if (!FileExists("assets/fonts/font.ttf")) {
        printf("Warning: Font file not found: assets/fonts/font.ttf\n");
    }
    
    if (!FileExists("assets/fonts/sdf.fs")) {
        printf("Warning: Shader file not found: assets/fonts/sdf.fs\n");
    }
}
