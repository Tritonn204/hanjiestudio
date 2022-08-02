#ifndef Viewer_hpp
#define Viewer_hpp

#include <SDL2/SDL.h>

#define VIEWER_HINTS 0

class Viewer {
public:
  int largestRow = 1;
  int largestCol = 1;

  int cellSize;

  float outlineWeight = 5;
  float clusterWeight = 3.5f;
  float lineWeight = 2.1;
  float fontSize = 40;

  //UI Toggle bindMouseControls
  bool showHints;

  //Theme colours
  SDL_Color borderColor = SDL_Color{80, 80, 80, 255};
  SDL_Color lineColor = SDL_Color{120, 120, 120, 60};
  SDL_Color clusterColor = SDL_Color{0, 70, 128, 100};
  SDL_Color cellColor = SDL_Color{0, 0, 0, 255};
  SDL_Color blankColor = SDL_Color{255, 255, 255, 0};
  SDL_Color hintColor = SDL_Color{0, 0, 0, 255};
  SDL_Color focusColor = SDL_Color{10, 60, 100, 200};

  Viewer();
  ~Viewer();

  SDL_Color *cursorCell(SDL_Color c, int opacity);

  void init(int cellSize = 48, bool sh = false);
  void toggle(int type);

private:

};

#endif // Viewer_hpp
