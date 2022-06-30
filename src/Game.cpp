#include "Game.hpp"
#include "Console.hpp"
#include "Nonogram.hpp"
#include "bmfont.hpp"
#include "SDL2/SDL_ttf.h"
#include "GFX.hpp"
#include "WinDPI.hpp"

Nonogram *nonogram = nullptr;
SDL_Event event;

Game::Game()
{

}
Game::~Game()
{
  cnt++;
}

void Game::init(const char *title, int xPos, int yPos, int width, int height, int flags)
{
  if (SDL_Init(SDL_INIT_EVERYTHING) == 0 )
  {
    log("SDL subsystems initialized!");
    float dpi, defaultDpi;
    MySDL_GetDisplayDPI(0, &dpi, &defaultDpi);
    mWidth = width;
    mHeight = height;
    width = int(width * dpi / defaultDpi);
    height = int(height * dpi / defaultDpi);
    window = SDL_CreateWindow(title, xPos, yPos, width, height, flags);
    if (window)
    {
      log("Window created!");

      mMouseFocus = true;
  		mKeyboardFocus = true;

      renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
      if (renderer)
      {
        SDL_SetRenderDrawColor(renderer,255,255,255,255);
        SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        log("Renderer created!");

        BitmapFont *font = new BitmapFont();
        font->init(renderer,"res/hints.fnt");

        nonogram = new Nonogram();
        nonogram->createContext(window, renderer);
        nonogram->linkMouse(&mouseX, &mouseY);
        nonogram->setFont(font);
        nonogram->init("Random", (rand()%15)+5, (rand()%15)+5);
        nonogram->fitToView();

        isRunning = true;
      } else {
        error(std::string("SDL_Init error! SDL Error: ") + std::string(SDL_GetError()));
      }
    }
  } else {
    error(std::string("SDL_Init error! SDL Error: ") + std::string(SDL_GetError()));
  }
}

void Game::handleEvents()
{
  while(SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        isRunning = false;
        break;
      case SDL_FINGERDOWN:
        std::cout << event.tfinger.x << std::endl;
        break;
      case SDL_WINDOWEVENT:
        if (event.window.windowID == mWindowID) {
      		switch( event.window.event )
      		{
      			//Window moved
      			case SDL_WINDOWEVENT_MOVED:
        			mWindowDisplayID = SDL_GetWindowDisplayIndex( window );
        			break;

      			//Window appeared
      			case SDL_WINDOWEVENT_SHOWN:
        			mShown = true;
        			break;

      			//Window disappeared
      			case SDL_WINDOWEVENT_HIDDEN:
        			mShown = false;
        			break;

      			//Get new dimensions and repaint
      			case SDL_WINDOWEVENT_SIZE_CHANGED:
        			mWidth = event.window.data1;
        			mHeight = event.window.data2;
        			SDL_RenderPresent( renderer );
        			break;

      			//Repaint on expose
      			case SDL_WINDOWEVENT_EXPOSED:
        			SDL_RenderPresent( renderer );
        			break;

      			//Mouse enter
      			case SDL_WINDOWEVENT_ENTER:
        			mMouseFocus = true;
        			break;

      			//Mouse exit
      			case SDL_WINDOWEVENT_LEAVE:
        			mMouseFocus = false;
        			break;

      			//Keyboard focus gained
      			case SDL_WINDOWEVENT_FOCUS_GAINED:
        			mKeyboardFocus = true;
        			break;

      			//Keyboard focus lost
      			case SDL_WINDOWEVENT_FOCUS_LOST:
        			mKeyboardFocus = false;
        			break;

      			//Window minimized
      			case SDL_WINDOWEVENT_MINIMIZED:
              mMinimized = true;
              break;

      			//Window maxized
      			case SDL_WINDOWEVENT_MAXIMIZED:
      			  mMinimized = false;
              break;

      			//Window restored
      			case SDL_WINDOWEVENT_RESTORED:
      			  mMinimized = false;
              break;

      			//Hide on close
      			case SDL_WINDOWEVENT_CLOSE:
        			SDL_HideWindow( window );
        			break;
      		}
        }
        break;
      case SDL_KEYDOWN:
        {
          //Display change flag
          switch( event.key.keysym.sym ){
            case SDLK_SPACE:
              nonogram->init("Random", (rand()%30)+10, (rand()%30)+10);
              nonogram->fitToView();
              break;
            case SDLK_RIGHT:
              break;
            case SDLK_LEFT:
              break;
            case SDLK_DOWN:
              screenshot("screenshot.png",renderer,window);
              break;
            }

            break;
        }
      case SDL_KEYUP:
        break;
      case SDL_MOUSEMOTION:
        SDL_GetMouseState( &mouseX, &mouseY );
        break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        break;
    }
  }
}

void Game::update()
{
  handleEvents();
  nonogram->fitToView();
}

void Game::render()
{
  SDL_SetRenderDrawColor(renderer,255,255,255,255);
  SDL_RenderClear(renderer);

  int width;
  int height;
  SDL_GetWindowSize(window, &width, &height);

  float scaleX;
  float scaleY;
  SDL_RenderGetScale(renderer, &scaleX, &scaleY);

  int w = nonogram->getWidth();
  int h = nonogram->getHeight();

  nonogram->render(width/2/scaleX - w/2, height/2/scaleY - h/2);

  SDL_RenderPresent(renderer);
}

void Game::clean()
{
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  log("Game Cleaned");
}
