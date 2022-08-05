#include "Game.hpp"
#include "Console.hpp"
#include "Nonogram.hpp"
#include "Viewer.hpp"
#include "bmfont.hpp"
#include "SDL2/SDL_ttf.h"
#include "GFX.hpp"
#include "WinDPI.hpp"
#include "nfd.hpp"

#include "GUI.hpp"

#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_syswm.h>
#include <windows.h>

Nonogram *nonogram = nullptr;
SDL_Event event;

typedef struct {
  Nonogram *n;
} SolveData;

int solvePuzzle(void* data)
{
  SolveData *tdata = (SolveData*)data;
  tdata->n->solvePuzzle();
  free(tdata);
  return 0;
}

//sadsa

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
    if (NFD_Init() != NFD_OKAY) {
      error("Failed to initiate NFD");
      return;
    }

    float dpi, defaultDpi;
    MySDL_GetDisplayDPI(0, &dpi, &defaultDpi);
    mWidth = width;
    mHeight = height;

    dpiScale = dpi / defaultDpi;

    width = width * dpiScale;
    height = height * dpiScale;
    window = SDL_CreateWindow(title, xPos, yPos, width, height, flags);
    if (window)
    {
      log("Window created!");

      mMouseFocus = true;
      mKeyboardFocus = true;

      renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
      if (renderer)
      {
        GUI *testGui = new GUI();
        testGui->createContext(renderer, window, "res/tileset.png");
        testGui->linkDpi(&dpiScale);
        testGui->init(0,0,400,300, true);

        unsigned weights[] = {1,4};
        KW_Font *baseFont = testGui->loadFont("res/pdfhints.ttf", 18);
        KW_Widget *editBox = testGui->addEditbox(80,100,32,35,32,"Name:", "", weights);
        KW_Font *editBoxFont = testGui->loadFont("res/titlefont.ttf", 18);

        KW_SetFont(testGui->getGui(), baseFont);
        KW_SetEditboxFont(editBox, editBoxFont);

        widgets.push_back(testGui);

        SDL_SetRenderDrawColor(renderer,255,255,255,255);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        log("Renderer created!");

        BitmapFont *font = new BitmapFont();
        font->init(renderer,"res/hints.fnt");

        nonogram = new Nonogram();
        nonogram->createContext(window, renderer, &zoom);
        nonogram->linkMouse(&mouseX, &mouseY);
        nonogram->setFont(font);
        nonogram->init("Random", 10, 10);
        nonogram->linkDpi(&dpiScale);
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
          space = true;
          break;
          case SDLK_RIGHT:
          nonogram->init("Random", (rand()%45)+5, (rand()%45)+5);
          nonogram->fitToView();
          break;
          case SDLK_LEFT:

          break;
          case SDLK_DOWN:
          {
            //SolveData *data;
            //data->n = nonogram;
            //SDL_Thread* solveThread = SDL_CreateThread(solvePuzzle, "Solver", data);
            //SDL_DetachThread(solveThread);
            //nonogram->savePuzzleTxt();
            nonogram->solvePuzzle();
            //nonogram->testPrint();
            break;
          }
          case SDLK_UP:
          nonogram->toggle(VIEWER_HINTS);
          nonogram->fitToView();
          break;
        }
        default:
        break;
      }
      case SDL_KEYUP:
      switch( event.key.keysym.sym ){
        case SDLK_SPACE:
        space = false;
        panning = false;
        break;
        default:
        break;
      }
      break;
      case SDL_MOUSEWHEEL:
      {
        float scaleX, scaleY;
        SDL_RenderGetScale(renderer, &scaleX, &scaleY);

        int W, H;
        SDL_GetRendererOutputSize(renderer, &W, &H);
        int ruler = std::min(W,H);

        float max = ((float)ruler/nonogram->getCellSize())/(W > H ? scaleY : scaleX)/5;

        zoom += (float)event.wheel.y/10.0f;
        zoom = std::min(zoom,max);
        zoom = std::max(zoom,(float)0.66);
      }
      break;
      case SDL_MOUSEMOTION:
      {
        float scaleX, scaleY;
        SDL_RenderGetScale(renderer, &scaleX, &scaleY);
        SDL_GetMouseState( &mouseX, &mouseY );
        if (panning) {
          cam.x += event.motion.xrel/scaleX;
          cam.y += event.motion.yrel/scaleY;
        }
        break;
      }
      case SDL_MOUSEBUTTONDOWN:
      {
        if (space) {
          if (event.button.button == SDL_BUTTON_LEFT) {
            panning = true;
          }
        } else {
          float scaleX, scaleY;
          SDL_RenderGetScale(renderer, &scaleX, &scaleY);
          if (nonogram->contains(mouseX/scaleX, mouseY/scaleY))
          nonogram->mouseEvent(event);
        }
        break;
      }
      case SDL_MOUSEBUTTONUP:
      nonogram->mouseEvent(event);
      if (event.button.button == SDL_BUTTON_LEFT) {
        panning = false;
      }
      break;
    }
  }
}

void Game::update()
{
  float dpi, defaultDpi;
  int index = SDL_GetWindowDisplayIndex( window );
  MySDL_GetDisplayDPI(index, &dpi, &defaultDpi);
  dpiScale = dpi / defaultDpi;
  handleEvents();
  nonogram->fitToView();
  nonogram->update();
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

  scaleX *= zoom;
  scaleY *= zoom;

  int w = nonogram->getWidth();
  int h = nonogram->getHeight();

  log(cam.x);

  nonogram->setPosition(
    (width/2)*zoom/scaleX - (w/2)*zoom + cam.x,
    (height/2)*zoom/scaleY  - (h/2)*zoom + cam.y
  );
  nonogram->render();

  for(size_t i = 0; i < widgets.size(); i++) {
    widgets[i]->update();
    widgets[i]->render();
  }

  SDL_RenderPresent(renderer);
}

void Game::clean()
{
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  NFD_Quit();
  for(size_t i = 0; i < widgets.size(); i++) {
    widgets[i]->quit();
  }
  SDL_Quit();
  log("Game Cleaned");
}
