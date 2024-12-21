#include "font.h"

Font_t font_load(char *ttf_filename, char *shader_filename) {
  Font font = {0};
  Shader shader;
  int fileSize = 0;
  unsigned char *fileData = LoadFileData(ttf_filename, &fileSize);

  // SDF font generation from TTF font
  font.baseSize = 32;
  font.glyphCount = 95;
  // Parameters > font size: 16, no glyphs array provided (0), glyphs count: 0
  // (defaults to 95)
  font.glyphs = LoadFontData(fileData, fileSize, font.baseSize, 0, 0, FONT_SDF);
  // Parameters > glyphs count: 95, font size: 16, glyphs padding in image: 0
  // px, pack method: 1 (Skyline algorythm)
  Image atlas =
      GenImageFontAtlas(font.glyphs, &font.recs, 95, font.baseSize, 0, 1);
  font.texture = LoadTextureFromImage(atlas);
  UnloadImage(atlas);

  UnloadFileData(fileData); // Free memory from loaded file

  // Load SDF required shader (we use default vertex shader)
  shader = LoadShader(0, TextFormat(shader_filename, 330));
  SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR); // Required for

  return (Font_t){font, shader};
}
