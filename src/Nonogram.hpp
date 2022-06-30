#ifndef Nonogram_hpp
#define Nonogram_hpp

#include <SDL2/SDL.h>
#include "Console.hpp"
#include <iostream>
#include <vector>
#include "bmfont.hpp"

class Viewer {
public:
  int largestRow = 1;
  int largestCol = 1;

  int cellSize = 48;

  float outlineWeight = 5;
  float clusterWeight = 3.5f;
  float lineWeight = 2.1;
  float fontSize = 40;

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

  void init(int cellSize);

private:

};

class Nonogram {
public:
  const char* name;

  int width;
  int height;

  bool isValid;

  Nonogram();
  ~Nonogram();

  void init(const char* title, int sizeX, int sizeY);

  void update();
  void refresh();
  void render(int posX, int posY);
  void createContext(SDL_Window *w, SDL_Renderer *r) {window = w; renderer = r;}
  void loadCells(int **c, int sizeX, int sizeY);
  void randomize();
  void fitToView();
  void linkMouse(int *mX, int *mY) {mouseX = mX; mouseY = mY;}

  int getWidth();
  int getHeight();

  void setCell(int xPos, int yPos, int value);
  void setFont(BitmapFont *f) { font = f; }

  std::vector<std::vector<int>> getCells() { return cells; }
  void clearCells();
  void validate();

  void exportPuzzle();

private:
  SDL_Window *window;
  SDL_Renderer *renderer;

  int *mouseX;
  int *mouseY;

  BitmapFont *font;

  std::vector<std::vector<int>> columns;
  std::vector<std::vector<int>> rows;

  std::vector<std::vector<int>> cells;
};

#endif /* Nonogram_hpp */
