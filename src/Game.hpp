#ifndef Game_hpp
#define Game_hpp

#include <iostream>
#include <SDL2/SDL.h>
#include "Console.hpp"

class Game {
public:
  Game();
  ~Game();

  void init(const char* title, int xPos, int yPos, int width, int height, int flags);

  void handleEvents();
  void update();
  void render();
  void clean();

  //Window dimensions
  int getWidth();
  int getHeight();

  //Window focii
  bool hasMouseFocus();
  bool hasKeyboardFocus();
  bool isMinimized();
  bool isShown();

  bool running() { return isRunning; };

private:
  int cnt;
  bool isRunning;

  int mouseX;
  int mouseY;

  //Window data
  SDL_Window *window;
  SDL_Renderer *renderer;
  int mWindowID;
  int mWindowDisplayID;

  //Window dimensions
  int mWidth;
  int mHeight;

  //Window focus
  bool mMouseFocus;
  bool mKeyboardFocus;
  bool mFullScreen;
  bool mMinimized;
  bool mShown;
};

#endif /* Game_hpp */
