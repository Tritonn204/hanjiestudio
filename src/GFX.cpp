#include "GFX.hpp"
#include <SDL2/SDL.h>
#include <cmath>
#include <vector>
#include <iostream>

#include "ImageIO.hpp"

int grct0[4] = {0,0,1,1};
int grct1[4] = {0,1,1,0};
int grct2[4] = {1,1,0,0};
int grct3[4] = {1,0,0,1};

void drawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, float weight, SDL_Color c)
{
  float dx = x2-x1;
  float dy = y2-y1;
  float length = hypot(dx,dy);

  float offsetX = weight*dy/(length*2.0f);
  float offsetY = weight*dx/(length*2.0f);

  std::vector<SDL_Vertex> t1 =
  {
    {SDL_FPoint {x1-offsetX, y1-offsetY}, c, SDL_FPoint{ 0 }},
    {SDL_FPoint {x1+offsetX, y1+offsetY}, c, SDL_FPoint{ 0 }},
    {SDL_FPoint {x2+offsetX, y2+offsetY}, c, SDL_FPoint{ 0 }},
  };

  std::vector<SDL_Vertex> t2 =
  {
    {SDL_FPoint {x2+offsetX, y2-offsetY}, c, SDL_FPoint{ 0 }},
    {SDL_FPoint {x1-offsetX, y1-offsetY}, c, SDL_FPoint{ 0 }},
    {SDL_FPoint {x2-offsetX, y2+offsetY}, c, SDL_FPoint{ 0 }},
  };

  if (SDL_RenderGeometry(renderer, NULL, t1.data(), t1.size(), NULL, 0) != 0) {
    std::cout << "SDL_RenderGeometry Failed! " << SDL_GetError() << std::endl;
  }

  if (SDL_RenderGeometry(renderer, NULL, t2.data(), t2.size(), NULL, 0) != 0) {
    std::cout << "SDL_RenderGeometry Failed! " << SDL_GetError() << std::endl;
  }
}

void gradientRect(SDL_Renderer *renderer, int x, int y, int w, int h, std::vector<SDL_Color> c, int dir)
{
  int steps = c.size()-1;
  int chunkSize = dir == GRECT_RIGHT || dir == GRECT_LEFT ? w/steps : h/steps;

  std::vector<SDL_Vertex> t1;
  std::vector<SDL_Vertex> t2;

  for(int i = 0; i < steps; i++) {
    if (dir == GRECT_RIGHT || dir == GRECT_LEFT) {
      t1 = {
        SDL_Vertex{SDL_FPoint{x+chunkSize*(i+grct0[dir]), y}, c[i+grct0[dir]], SDL_FPoint{ 0 }},
        SDL_Vertex{SDL_FPoint{x+chunkSize*(i+grct1[dir]), y+h}, c[i+grct1[dir]], SDL_FPoint{ 0 }},
        SDL_Vertex{SDL_FPoint{x+chunkSize*(i+grct2[dir]), y+h}, c[i+grct2[dir]], SDL_FPoint{ 0 }}
      };
      t2 = {
        SDL_Vertex{SDL_FPoint{x+chunkSize*(i+grct2[dir]), y+h}, c[i+grct2[dir]], SDL_FPoint{ 0 }},
        SDL_Vertex{SDL_FPoint{x+chunkSize*(i+grct3[dir]), y}, c[i+grct3[dir]], SDL_FPoint{ 0 }},
        SDL_Vertex{SDL_FPoint{x+chunkSize*(i+grct0[dir]), y}, c[i+grct0[dir]], SDL_FPoint{ 0 }}
      };
    } else {
      t1 = {
        SDL_Vertex{SDL_FPoint{x, y+chunkSize*(i+grct0[dir])}, c[i+grct0[dir]], SDL_FPoint{ 0 }},
        SDL_Vertex{SDL_FPoint{x, y+chunkSize*(i+grct1[dir])}, c[i+grct1[dir]], SDL_FPoint{ 0 }},
        SDL_Vertex{SDL_FPoint{x+w, y+chunkSize*(i+grct2[dir])}, c[i+grct2[dir]], SDL_FPoint{ 0 }}
      };
      t2 = {
        SDL_Vertex{SDL_FPoint{x+w, y+chunkSize*(i+grct2[dir])}, c[i+grct2[dir]], SDL_FPoint{ 0 }},
        SDL_Vertex{SDL_FPoint{x+w, y+chunkSize*(i+grct3[dir])}, c[i+grct3[dir]], SDL_FPoint{ 0 }},
        SDL_Vertex{SDL_FPoint{x, y+chunkSize*(i+grct0[dir])}, c[i+grct0[dir]], SDL_FPoint{ 0 }}
      };
    }

    if (SDL_RenderGeometry(renderer, NULL, t1.data(), t1.size(), NULL, 0) != 0) {
      std::cout << "SDL_RenderGeometry Failed! " << SDL_GetError() << std::endl;
    }

    if (SDL_RenderGeometry(renderer, NULL, t2.data(), t2.size(), NULL, 0) != 0) {
      std::cout << "SDL_RenderGeometry Failed! " << SDL_GetError() << std::endl;
    }
  }
}

float getAngle(SDL_FPoint a, SDL_FPoint b)
{
  float dx = b.x-a.x;
  float dy = b.y-a.y;
  return atan2(dy,dx);
}

void screenshot(const char* file_name, SDL_Renderer* renderer, SDL_Window *window)
{
  save_texture(file_name,renderer,SDL_CreateTextureFromSurface(renderer, SDL_GetWindowSurface(window)));
}

void traceShape(SDL_Renderer *renderer, SDL_FPoint* points, int num_points, float weight, SDL_Color c)
{
  bool loop =
    points[0].x == points[num_points-1].x &&
    points[0].y == points[num_points-1].y;

  for (int i = 0; i < num_points-1; i++) {

    SDL_FPoint lastPoint = i > 0 ? points[i-1] : (loop ? points[num_points-2] : points[i]);
    SDL_FPoint nextPoint = i < num_points-3 ? points[i+2] : (loop ? points[0] : points[i+1]);

    float x1 = points[i].x;
    float x2 = points[i+1].x;
    float y1 = points[i].y;
    float y2 = points[i+1].y;

    float dx = x2-x1;
    float dy = y2-y1;
    float length = hypot(dx,dy);

    float offsetX = weight*(dy)/(length*2.0f);
    float offsetY = weight*(dx)/(length*2.0f);

    float a = atan2(dy,dx);
    float vX = cos(a);
    float vY = sin(a);

    float lastAngle = getAngle(lastPoint, points[i]);
    float nextAngle = getAngle(points[i+1], nextPoint);

    float lA = weight*tan((a-lastAngle)/2);
    float lB = weight*tan((a-nextAngle)/2);
    float mX1 = lA*vX;
    float mY1 = lA*vY;
    float mX2 = lB*vX;
    float mY2 = lB*vY;

    std::vector<SDL_Vertex> t1 =
    {
      {SDL_FPoint {x1-offsetX-mX1/2, y1-offsetY+mY1/2}, c, SDL_FPoint{ 0 }},
      {SDL_FPoint {x1+offsetX+mX1/2, y1+offsetY-mY1/2}, c, SDL_FPoint{ 0 }},
      {SDL_FPoint {x2+offsetX+mX2/2, y2+offsetY-mY2/2}, c, SDL_FPoint{ 0 }},
    };

    std::vector<SDL_Vertex> t2 =
    {
      {SDL_FPoint {x2+offsetX-mX2/2, y2-offsetY-mY2/2}, c, SDL_FPoint{ 0 }},
      {SDL_FPoint {x1-offsetX-mX1/2, y1-offsetY+mY1/2}, c, SDL_FPoint{ 0 }},
      {SDL_FPoint {x2-offsetX+mX2/2, y2+offsetY+mY2/2}, c, SDL_FPoint{ 0 }},
    };

    if (SDL_RenderGeometry(renderer, NULL, t1.data(), t1.size(), NULL, 0) != 0) {
      std::cout << "SDL_RenderGeometry Failed! " << SDL_GetError() << std::endl;
    }

    if (SDL_RenderGeometry(renderer, NULL, t2.data(), t2.size(), NULL, 0) != 0) {
      std::cout << "SDL_RenderGeometry Failed! " << SDL_GetError() << std::endl;
    }
  }
}
