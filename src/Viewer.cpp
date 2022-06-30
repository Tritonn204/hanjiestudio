#include "Nonogram.hpp"

Viewer::Viewer()
{

}

Viewer::~Viewer()
{

}

void Viewer::init(int cellSize)
{
  cellSize = 32;
}

SDL_Color cursorCache;

SDL_Color *Viewer::cursorCell(SDL_Color c, int opacity)
{
  cursorCache = SDL_Color{255-c.r,255-c.g,255-c.b,opacity};
  return &cursorCache;
}
