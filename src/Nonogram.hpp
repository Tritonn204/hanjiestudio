#ifndef Nonogram_hpp
#define Nonogram_hpp

#include <SDL2/SDL.h>
#include "Console.hpp"
#include <iostream>
#include <vector>
#include "bmfont.hpp"
#include "mouseobject.hpp"
#include "Texture.hpp"
#include "Viewer.hpp"
#include "Solver.hpp"

class Nonogram {
public:
  const char* name;

  int width;
  int height;

  bool isValid;
  bool paused;

  std::vector<bool> toggles = std::vector<bool>(50,false);

  std::vector<std::vector<int>> columns;
  std::vector<std::vector<int>> rows;

  std::vector<std::vector<int>> cells;

  Nonogram();
  ~Nonogram();

  void init(const char* title, int sizeX, int sizeY);

  void bindMouseControls();

  void update();
  void refresh();
  void render();
  void createContext(SDL_Window *w, SDL_Renderer *r, float *z)
  {
    window = w;
    renderer = r;
    zoom = z;
  }
  void loadCells(int **c, int sizeX, int sizeY);
  void randomize();
  void fitToView();
  void linkMouse(int *mX, int *mY) {mouseX = mX; mouseY = mY;}
  void setPosition(int x, int y);
  void scaledMousePos(int *qX, int *qY);
  void cellPos(int *qX, int *qY);
  Texture *generateBitmap();
  void savePuzzle();
  void savePuzzleTxt();
  void loadPuzzle();

  int getCellSize() {return viewer->cellSize;}
  int getLargestRow() {return viewer->largestRow;}
  int getLargestCol() {return viewer->largestCol;}

  void toggle(int type) {
    viewer->toggle(type);
    toggles[type] = !toggles[type];
  }

  void onClick(SDL_Event e);
  void onRelease(SDL_Event e);

  void mouseEvent(SDL_Event e) {mouseHandler->mouseEvent(e);}
  bool contains(int x, int y);

  int getWidth();
  int getHeight();

  void setCell(int xPos, int yPos, int value);
  void setFont(BitmapFont *f) { font = f; }

  std::vector<std::vector<int>> getCells() { return cells; }
  std::vector<std::vector<int>> fromTexture(Texture *img);

  void clearCells();
  void validate();

  void solvePuzzle();

  void exportPuzzle();

private:
  SDL_Window *window;
  SDL_Renderer *renderer;
  Viewer *viewer;
  MouseObject *mouseHandler;
  Solver *solver;

  float *zoom;

  int *mouseX;
  int *mouseY;

  int X;
  int Y;

  int currentButton;

  BitmapFont *font;
};

#endif /* Nonogram_hpp */
