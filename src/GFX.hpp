#ifndef GFX_hpp
#define GFX_hpp

#include <cmath>
#include <vector>
#include <SDL2/SDL.h>

#define PI 3.14159265358979323846
#define GRECT_RIGHT 0
#define GRECT_DOWN 1
#define GRECT_LEFT 2
#define GRECT_UP 3

void drawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, float weight, SDL_Color c);
void traceShape(SDL_Renderer *renderer, SDL_FPoint* points, int num_points, float weight, SDL_Color c);
void gradientRect(SDL_Renderer *renderer, int x, int y, int w, int h, std::vector<SDL_Color> c, int dir);
void screenshot(const char* file_name, SDL_Renderer* renderer, SDL_Window *window);

#endif /* GFX_hpp */
