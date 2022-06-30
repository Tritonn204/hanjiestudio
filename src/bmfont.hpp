#ifndef bmfont_h
#define bmfont_h

#include "SDL2/SDL.h"
#include <string>
#include <map>
#include <vector>

//font alignment flags
#define FONT_ALIGN_LEFT 0
#define FONT_ALIGN_CENTER 1
#define FONT_ALIGN_RIGHT 2

//max amount of pages per font
#define BFONT_PAGE_BUFFER_SIZE 25

class BitmapFont {
public:
  BitmapFont();
  ~BitmapFont();

  struct Glyph {
    int x;
    int y;
    int w;
    int h;
    int xOffset;
    int yOffset;
    int xAdvance;
    int page;
  };

  void init(SDL_Renderer *r, const char* file);
  void readAtlas(SDL_Renderer *renderer, const char* path);
  void draw(SDL_Renderer *r, float x, float y, float size, const char* string, SDL_Color c, int alignment);
  void drawScaled(SDL_Renderer *r, float scaler, float x, float y, float size, const char* text, SDL_Color color, int alignment);

private:
  SDL_Texture *pages[BFONT_PAGE_BUFFER_SIZE];
  std::map<int, Glyph> glyphs;
  std::map<int, std::map<int, int>> kerning;
};

#endif // bmfont_h
