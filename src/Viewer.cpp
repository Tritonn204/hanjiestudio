#include "Nonogram.hpp"
#include "Viewer.hpp"

Viewer::Viewer()
{

}

Viewer::~Viewer()
{

}

void Viewer::init(int cs, bool sh)
{
  showHints = sh;
  cellSize = cs;
}

void Viewer::toggle(int type)
{
  switch(type) {
    case VIEWER_HINTS:
      showHints = !showHints;
      break;
    default:
      break;
  }
}

SDL_Color cursorCache;

SDL_Color *Viewer::cursorCell(SDL_Color c, int opacity)
{
  cursorCache = SDL_Color{
    static_cast<uint8_t>(255-c.r),
    static_cast<uint8_t>(255-c.g),
    static_cast<uint8_t>(255-c.b),
    static_cast<uint8_t>(opacity)
  };
  return &cursorCache;
}
