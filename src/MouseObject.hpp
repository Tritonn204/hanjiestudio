#ifndef mouseobject_hpp
#define mouseobject_hpp

#include "SDL2/SDL.h"
#include <stdio.h>
#include <vector>
#include <functional>

class MouseObject {
public:
  MouseObject();
  ~MouseObject();

  typedef std::function<void(SDL_Event e)> Callback;
  std::map<int, Callback> callbacks;


  void setBounds(SDL_Rect r) {bounds = r;}
  void mouseEvent(SDL_Event event) {callbacks[event.type](event);}
  void addCall(int i, Callback c) {callbacks[i] = c;}
  bool contains(int x, int y) {
    return (
      x >= bounds.x &&
      x < bounds.x+bounds.h &&
      y >= bounds.y &&
      y < bounds.y+bounds.h
    );
  }

private:
  SDL_Rect bounds;
};

#endif // mouseobject_hpp
