#ifndef gui_hpp
#define gui_hpp

#include <SDL2/SDL.h>

#include "KiWi/KW_gui.h"
#include "KiWi/KW_frame.h"
#include "KiWi/KW_editbox.h"
#include "KiWi/KW_label.h"
#include "KiWi/KW_button.h"
#include "KiWi/KW_renderdriver_sdl2.h"

#include <vector>

class GUI
{
public:
  GUI();
  ~GUI();

  void init(int x, int y, int w, int h, bool centered);
  void createContext(SDL_Renderer *r, SDL_Window *w, const char* tiles);

  void linkDpi(float *dScale) {dpiScale = dScale;}

  void update();
  void render();

  void moveTo(int x, int y);
  void resize(int w, int h);

  KW_GUI *getGui() {return gui;}

  KW_Font *loadFont(const char *path, int fontSize);
  KW_Widget *addButton(int x, int y, int w, int h, const char* label, void(*callback)(KW_Widget * widget, int b));
  KW_Widget *addLabel(int x, int y, int w, int h, const char* text);
  KW_Widget *addEditbox(
    int y,
    int w, int h,
    int lW, int lH,
    const char* label,
    const char* def,
    unsigned weights[]
  );

  void quit();

private:
  SDL_Window *window;
  SDL_Renderer *renderer;
  KW_Widget *frame;

  std::vector<KW_Font*> memFonts;

  float *dpiScale;

  int memPosX;
  int memPosY;
  int memW;
  int memH;

  KW_Rect framerect;
  KW_Rect windowrect;

  KW_RenderDriver * driver;
  KW_Surface * tileset;
  KW_GUI * gui;
};

#endif // gui_hpp
