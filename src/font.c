#include "font.h"
#include "error.h"
#include "logger.h"
#include <stdio.h>

Font_t font_load(char *ttf_filename, char *shader_filename) {
  Font font = {0};
  Shader shader = {0};
  
  LOG_INFO("Loading font from %s with shader %s", ttf_filename, shader_filename);
  
  // Check if file exists before trying to load it
  if (FileExists(ttf_filename)) {
    LOG_DEBUG("Font file exists: %s", ttf_filename);
    int fileSize = 0;
    unsigned char *fileData = LoadFileData(ttf_filename, &fileSize);
    
    if (fileData != NULL && fileSize > 0) {
      LOG_DEBUG("Loaded font file data: %d bytes", fileSize);
      
      // SDF font generation from TTF font
      font.baseSize = 64; // Increased base size to accommodate larger characters
      font.glyphCount = 1200; // Reduced from 512 to avoid rect packing issues
      LOG_DEBUG("Generating SDF font with base size %d, glyph count %d", font.baseSize, font.glyphCount);
      
      // Parameters > font size: font.baseSize, no glyphs array provided (0), glyphs count: 0
      // (defaults to 95)
      font.glyphs = LoadFontData(fileData, fileSize, font.baseSize, 0, font.glyphCount, FONT_SDF);
      
      if (font.glyphs != NULL) {
        LOG_DEBUG("Font data loaded successfully");
        
        // Parameters > glyphs count: font.glyphCount, font size: font.baseSize, glyphs padding: 4
        // px, pack method: 1 (Skyline algorithm)
        LOG_DEBUG("Generating font atlas");
        
        // Try with a reasonable atlas size first
        Image atlas = {0};
        
        // Temporarily disable raylib logging to avoid warnings
        
        // Try with increasing atlas padding until successful
        int padding = 4;
        bool success = false;
        
        // First try with requested glyph count
        atlas = GenImageFontAtlas(font.glyphs, &font.recs, font.glyphCount, font.baseSize, padding, 1);
        if (atlas.data != NULL) {
            success = true;
        }
        
        // If failed, try with fewer glyphs
        if (!success) {
            LOG_WARNING("Failed to generate font atlas with %d glyphs, trying with fewer", font.glyphCount);
            font.glyphCount = 128; // Fallback to ASCII extended
            atlas = GenImageFontAtlas(font.glyphs, &font.recs, font.glyphCount, font.baseSize, padding, 1);
            
            if (atlas.data != NULL) {
                success = true;
            }
        }
        
        // Last resort: try with basic ASCII only
        if (!success) {
            LOG_WARNING("Still failed, falling back to ASCII only (95 glyphs)");
            font.glyphCount = 95; // Fallback to ASCII only
            atlas = GenImageFontAtlas(font.glyphs, &font.recs, 95, font.baseSize, padding, 1);
        }
        
        // Restore previous log level
        if (atlas.data != NULL) {
          LOG_DEBUG("Font atlas generated successfully: %dx%d", atlas.width, atlas.height);
          font.texture = LoadTextureFromImage(atlas);
          LOG_DEBUG("Font texture loaded: ID %u", font.texture.id);
          UnloadImage(atlas);
          
          // Load SDF required shader (we use default vertex shader)
          if (FileExists(shader_filename)) {
            LOG_DEBUG("Shader file exists: %s", shader_filename);
            shader = LoadShader(0, shader_filename);
            if (shader.id == 0) {
              LOG_WARNING("Failed to load shader: %s", shader_filename);
              ERROR_SET(ERROR_SHADER_LOAD, ERROR_WARNING, "Failed to load shader");
            } else {
              LOG_DEBUG("Shader loaded successfully: ID %u", shader.id);
              SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
              LOG_DEBUG("Set texture filter to bilinear");
            }
          } else {
            LOG_WARNING("Shader file not found: %s", shader_filename);
            ERROR_SET(ERROR_FILE_NOT_FOUND, ERROR_WARNING, "Shader file not found");
          }
        } else {
          LOG_WARNING("Failed to generate font atlas");
          ERROR_SET(ERROR_FONT_LOAD, ERROR_WARNING, "Failed to generate font atlas");
        }
      } else {
        LOG_WARNING("Failed to load font data from %s", ttf_filename);
        ERROR_SET(ERROR_FONT_LOAD, ERROR_WARNING, "Failed to load font data");
      }
      
      UnloadFileData(fileData); // Free memory from loaded file
      LOG_DEBUG("Unloaded font file data");
    } else {
      LOG_WARNING("Failed to load font file data: %s", ttf_filename);
      ERROR_SET(ERROR_FILE_ACCESS, ERROR_WARNING, "Failed to load font file data");
    }
  } else {
    LOG_WARNING("Font file not found: %s", ttf_filename);
    ERROR_SET(ERROR_FILE_NOT_FOUND, ERROR_WARNING, "Font file not found");
  }
  
  if (font.texture.id > 0) {
    LOG_INFO("Font loaded successfully from %s", ttf_filename);
  } else {
    LOG_ERROR("Failed to load font from %s", ttf_filename);
  }
  
  return (Font_t){font, shader};
}

// Returns the raw glyph advance for the given codepoint.
float font_glyph_advance_get(const Font *font, int codepoint) {
  int glyph_index = GetGlyphIndex(*font, codepoint);
  
  // Check if the glyph index is valid
  if (glyph_index < 0 || glyph_index >= font->glyphCount) {
    // Return a default advance for missing glyphs
    return font->baseSize * 0.5f;
  }
  
  if (font->glyphs[glyph_index].advanceX != 0)
    return font->glyphs[glyph_index].advanceX;
  else
    return font->recs[glyph_index].width + font->glyphs[glyph_index].offsetX;
}

// Returns the scaled glyph advance for the given codepoint.
float font_glyph_advance_scaled_get(const Font *font, int codepoint,
                                           float scale, float font_base_size) {
  return (font_glyph_advance_get(font, codepoint) / font_base_size) * scale;
}
              
