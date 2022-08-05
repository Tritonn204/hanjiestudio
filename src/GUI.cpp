#include "GUI.hpp"

#include "KiWi/KW_gui.h"
#include "KiWi/KW_frame.h"
#include "KiWi/KW_editbox.h"
#include "KiWi/KW_label.h"
#include "KiWi/KW_button.h"
#include "KiWi/KW_renderdriver_sdl2.h"

#include "SDL2/sdl.h"

GUI::GUI()
{}

GUI::~GUI()
{}

void GUI::createContext(SDL_Renderer *r, SDL_Window *w, const char* tiles)
{
  window = w;
  renderer = r;
  driver = KW_CreateSDL2RenderDriver(r, w);
  tileset = KW_LoadSurface(driver, tiles);
  gui = KW_Init(driver, tileset);
}

void GUI::init(int x, int y, int w, int h, bool centered)
{
  int wX, wY, wW, wH;

  SDL_GetWindowPosition(window, &wX, &wY);
  SDL_GetWindowSize(window, &wW, &wH);

  windowrect = KW_Rect{ .x = wX, .y = wY, .w = wW, .h = wH };
  framerect = KW_Rect{ .x = (*dpiScale)*memPosX, .y = (*dpiScale)*memPosY, .w = (*dpiScale)*memW, .h = (*dpiScale)*memH };

  moveTo(x, y);
  resize(w, h);

  if (centered) KW_RectCenterInParent(&windowrect, &framerect);
  frame = KW_CreateFrame(gui, NULL, &framerect);
}

KW_Font *GUI::loadFont(const char *path, int fontSize)
{
  KW_Font *font = KW_LoadFont(driver, path, fontSize*(*dpiScale));
  memFonts.push_back(font);
  return font;
}

void GUI::moveTo(int x, int y)
{
  memPosX = x;
  memPosY = y;
  framerect = KW_Rect{ .x = (*dpiScale)*memPosX, .y = (*dpiScale)*memPosY, .w = (*dpiScale)*memW, .h = (*dpiScale)*memH };
}

void GUI::resize(int w, int h)
{
  memW = w;
  memH = h;
  framerect = KW_Rect{ .x = (*dpiScale)*memPosX, .y = (*dpiScale)*memPosY, .w = (*dpiScale)*memW, .h = (*dpiScale)*memH };
}

KW_Widget *GUI::addButton(int x, int y, int w, int h, const char* label, void(*callback)(KW_Widget * widget, int b))
{
  KW_Rect buttonrect = KW_Rect{ .x = (*dpiScale)*x, .y = (*dpiScale)*y, .w = (*dpiScale)*w, .h = (*dpiScale)*h };
  KW_Widget *button = KW_CreateButtonAndLabel(gui, frame, label, &buttonrect);
  KW_AddWidgetMouseDownHandler(button, callback);
  return button;
}

KW_Widget *GUI::addLabel(int x, int y, int w, int h, const char* text)
{
  KW_Rect labelrect = KW_Rect{ .x = (*dpiScale)*x, .y = (*dpiScale)*y, .w = (*dpiScale)*w, .h = (*dpiScale)*h };
  KW_Widget *label = KW_CreateLabel(gui, frame, text, &labelrect);
  return label;
}

KW_Widget *GUI::addEditbox(
  int y,
  int w, int h,
  int lW, int lH,
  const char* label,
  const char* def,
  unsigned weights[]
)
{
  KW_Rect labelrect = KW_Rect{ .y = (*dpiScale)*y, .w = (*dpiScale)*lW, .h = (*dpiScale)*lH };
  KW_Rect editboxrect = KW_Rect{ .y = (*dpiScale)*y, .w = (*dpiScale)*w, .h = (*dpiScale)*h };
  KW_Rect *rects[] = { &labelrect, &editboxrect };
  KW_RectFillParentHorizontally(&framerect, rects, weights, 2*(*dpiScale), 32*(*dpiScale), KW_RECT_ALIGN_MIDDLE);
  KW_CreateLabel(gui, frame, label, &labelrect);
  KW_Widget *editBox = KW_CreateEditbox(gui, frame, def, &editboxrect);
  return editBox;
}

void GUI::update()
{
  KW_ProcessEvents(gui);
}

void GUI::render()
{
  float scaleX;
  float scaleY;
  SDL_RenderGetScale(renderer, &scaleX, &scaleY);
  SDL_RenderSetScale(renderer, 1, 1);
  KW_Paint(gui);
  SDL_RenderSetScale(renderer, scaleX, scaleY);
}

void GUI::quit()
{
  KW_Quit(gui);
  for(size_t i = 0; i < memFonts.size(); i++) {
      KW_ReleaseFont(driver, memFonts[i]);
  }
  KW_ReleaseSurface(driver, tileset);
  KW_ReleaseRenderDriver(driver);
}
