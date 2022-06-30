#include "Game.hpp"
#include "Console.hpp"
#include <ctime>
#include <SDL2/SDL.h>
#include <windows.h>
#include "WinDPI.hpp"

Game *game = nullptr;

int init()
{
  srand(time(NULL));

  game = new Game();

  int w = GetSystemMetrics(SM_CXSCREEN);
  int h = GetSystemMetrics(SM_CYSCREEN);

  game->init("Nonogrammer v1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w/2, h/2, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI );
}

int main (int argc, char *argv[])
{
  init();

  while(game->running()) {
    game->update();
    game->render();
  }

  game->clean();

  return EXIT_SUCCESS;
}
