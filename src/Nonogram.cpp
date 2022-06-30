#include "Nonogram.hpp"
#include "Console.hpp"
#include "GFX.hpp"
#include "WinDPI.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdio>

Viewer *viewer = nullptr;

Nonogram::Nonogram()
{

}

Nonogram::~Nonogram()
{

}

void Nonogram::fitToView()
{
  int vw;
  int vh;
  SDL_GetWindowSize(window, &vw, &vh);

  int w = Nonogram::getWidth();
  int h = Nonogram::getHeight();

  float ri = w/h;
  float rs = vw/vh;

  float result = std::min(0.66f*(float)vw/(float)w, 0.8f*(float)vh/(float)h);

  SDL_RenderSetScale(renderer, result, result);
}

void Nonogram::refresh()
{
  //fill column numbers
  for(int i = 0; i < cells.size(); i++) {
    std::vector<int> col(0);
    int count = 0;
    for(int j = 0; j < cells[i].size(); j++) {
      if (cells[i][j] == 1) count++;
      if (cells[i][j] == 0 || j+1 >= cells[i].size()) {
        if (count > 0 || (j+1 >= cells[i].size() && col.size() == 0)) col.push_back(count);
        count = 0;
      }
    }
    columns[i] = col;
  }

  //fill row numbers
  for(int i = 0; i < cells[0].size(); i++) {
    std::vector<int> row(0);
    int count = 0;
    for(int j = 0; j < cells.size(); j++) {
      if (cells[j][i] == 1) count++;
      if (cells[j][i] == 0 || j+1 >= cells.size()) {
        if (count > 0 || (j+1 >= cells.size() && row.size() == 0)) row.push_back(count);
        count = 0;
      }
    }
    rows[i] = row;
  }

  //Adjust edge padding for solution numbers
  int temp = 0;
  for(int i = 0; i < rows.size(); i++) {
    if (rows[i].size() > temp) temp = rows[i].size();
  }
  viewer->largestRow = temp;

  temp = 0;
  for(int i = 0; i < columns.size(); i++) {
    if (columns[i].size() > temp) temp = columns[i].size();
  }
  viewer->largestCol = temp;
}

void Nonogram::init(const char* title, int sizeX, int sizeY)
{
  viewer = new Viewer();
  name = title;

  //Initialize puzzle cells
  cells = std::vector<std::vector<int>>(sizeX, std::vector<int> (sizeY, 0));

  //Initialize solution rows and columns
  rows = std::vector<std::vector<int>>(sizeY, std::vector<int> (1, 0));
  columns = std::vector<std::vector<int>>(sizeX, std::vector<int> (1, 0));

  Nonogram::randomize();
}

void Nonogram::randomize()
{
  for(int i = 0; i < cells.size(); i++) {
    for(int j = 0; j < cells[i].size(); j++) {
      cells[i][j] = rand()%2 < 1 ? 1 : 0;
    }
  }
  Nonogram::refresh();
}

int Nonogram::getWidth() {
  return cells.size()*viewer->cellSize + viewer->largestRow*viewer->cellSize;
}

int Nonogram::getHeight() {
  return cells[0].size()*viewer->cellSize + viewer->largestCol*viewer->cellSize;
}

void Nonogram::render(int posX, int posY)
{
  //HUD
  float scaleX, scaleY;
  SDL_RenderGetScale(renderer, &scaleX, &scaleY);

  float dpi, defaultDpi, dpiScaler;
  MySDL_GetDisplayDPI(SDL_GetWindowDisplayIndex(window), &dpi, &defaultDpi);

  dpiScaler = dpi/defaultDpi;

  posX -= (viewer->largestRow*viewer->cellSize)/4;

  std::vector<SDL_Color> gradientBuffer;
  std::vector<std::vector<int>> baseLines;
  std::vector<std::vector<int>> darkLines;
  //Render row dividers
  for(int i = 0; i <= rows.size(); i++) {
    bool clusterLine = false;
    if (i % 5 == 0) {
      darkLines.push_back({
        (int)(posX + viewer->largestRow*viewer->cellSize),
        (int)(posY + viewer->largestCol*viewer->cellSize + i*viewer->cellSize),
        (int)(posX + viewer->largestRow*viewer->cellSize + cells.size()*viewer->cellSize),
        (int)(posY + viewer->largestCol*viewer->cellSize + i*viewer->cellSize)
      });
    } else {
      baseLines.push_back({
        (int)(posX + viewer->largestRow*viewer->cellSize),
        (int)(posY + viewer->largestCol*viewer->cellSize + i*viewer->cellSize),
        (int)(posX + viewer->largestRow*viewer->cellSize + cells.size()*viewer->cellSize),
        (int)(posY + viewer->largestCol*viewer->cellSize + i*viewer->cellSize)
      });
    }

    if (i < rows.size()){
      bool highlight = false;
      gradientBuffer.clear();
      if (
        *mouseX/scaleX >= posX&&
        *mouseX/scaleX < posX + viewer->largestRow*viewer->cellSize+ cells.size()*viewer->cellSize &&
        *mouseY/scaleY >= posY + viewer->largestCol*viewer->cellSize + i*viewer->cellSize &&
        *mouseY/scaleY < posY + viewer->largestCol*viewer->cellSize + (i+1)*viewer->cellSize
      ) {
        gradientBuffer.push_back(SDL_Color{10,60,180,0});
        gradientBuffer.push_back(SDL_Color{10,60,180,50});
        highlight = true;
      } else if (i%2 == 0 && i < rows.size()){
        gradientBuffer.push_back(SDL_Color{0,0,0,0});
        gradientBuffer.push_back(SDL_Color{0,0,0,50});
      }

      if (gradientBuffer.size() > 0) gradientRect(
        renderer,
        posX, posY + viewer->largestCol*viewer->cellSize + i*viewer->cellSize,
        viewer->largestRow*viewer->cellSize, viewer->cellSize,
        gradientBuffer,
        GRECT_RIGHT
      );

      for(int h = 0; h < rows[i].size(); h++) {
        font->draw(
          renderer,
          posX + viewer->largestRow*viewer->cellSize - (h+1)*viewer->cellSize + viewer->cellSize/2,
          posY + viewer->largestCol*viewer->cellSize + i*viewer->cellSize + (viewer->cellSize-viewer->fontSize)/2,
          viewer->fontSize,
          const_cast<char*>((std::to_string(rows[i][rows[i].size()-h-1])).c_str()),
          highlight ? viewer->focusColor : viewer->borderColor,
          FONT_ALIGN_CENTER
        );
      }
    }
  }

  //Render column dividers
  for(int i = 0; i <= columns.size(); i++) {
    if (i % 5 == 0) {
      darkLines.push_back({
        (int)(posX + viewer->largestRow*viewer->cellSize + i*viewer->cellSize),
        (int)(posY + viewer->largestCol*viewer->cellSize),
        (int)(posX + viewer->largestRow*viewer->cellSize + i*viewer->cellSize),
        (int)(posY + viewer->largestCol*viewer->cellSize + cells[0].size()*viewer->cellSize)
      });
    } else {
      baseLines.push_back({
        (int)(posX + viewer->largestRow*viewer->cellSize + i*viewer->cellSize),
        (int)(posY + viewer->largestCol*viewer->cellSize),
        (int)(posX + viewer->largestRow*viewer->cellSize + i*viewer->cellSize),
        (int)(posY + viewer->largestCol*viewer->cellSize + cells[0].size()*viewer->cellSize)
      });
    }

    gradientBuffer.clear();
    gradientBuffer.push_back(SDL_Color{0,0,0,0});
    gradientBuffer.push_back(SDL_Color{0,0,0,50});

    if (i < columns.size()) {
      bool highlight = false;
      gradientBuffer.clear();
      if (
        *mouseY/scaleY >= posY&&
        *mouseY/scaleY < posY + viewer->largestCol*viewer->cellSize+ cells[0].size()*viewer->cellSize &&
        *mouseX/scaleX >= posX + viewer->largestRow*viewer->cellSize + i*viewer->cellSize &&
        *mouseX/scaleX < posX + viewer->largestRow*viewer->cellSize + (i+1)*viewer->cellSize
      ) {
        gradientBuffer.push_back(SDL_Color{10,60,180,0});
        gradientBuffer.push_back(SDL_Color{10,60,180,50});
        highlight = true;
      } else if (i%2 == 0 && i < columns.size()){
        gradientBuffer.push_back(SDL_Color{0,0,0,0});
        gradientBuffer.push_back(SDL_Color{0,0,0,50});
      }

      if (gradientBuffer.size() > 0) gradientRect(
        renderer,
        posX + viewer->largestRow*viewer->cellSize + i*viewer->cellSize, posY,
        viewer->cellSize, viewer->largestCol*viewer->cellSize,
        gradientBuffer,
        GRECT_DOWN
      );

      for(int h = 0; h < columns[i].size(); h++) {
        font->draw(
          renderer,
          posX + viewer->largestRow*viewer->cellSize + i*viewer->cellSize + viewer->cellSize/2,
          posY + viewer->largestCol*viewer->cellSize - (h+1)*viewer->cellSize,
          viewer->fontSize,
          const_cast<char*>((std::to_string(columns[i][columns[i].size()-h-1])).c_str()),
          highlight ? viewer->focusColor : viewer->borderColor,
          FONT_ALIGN_CENTER
        );
      }
    }
  }

  //Render filled cells
  SDL_Rect cell;
  SDL_Color *cBuffer;
  cell.w = viewer->cellSize;
  cell.h = viewer->cellSize;

  for(int i = 0; i < cells.size(); i++) {
    for(int j = 0; j < cells[0].size(); j++) {
      cBuffer = cells[i][j] == 1 ? &viewer->cellColor : &viewer->blankColor;
      SDL_SetRenderDrawColor(renderer, cBuffer->r, cBuffer->g, cBuffer->b, cBuffer->a);
      cell.x = posX + viewer->largestRow*viewer->cellSize + i*viewer->cellSize;
      cell.y = posY + viewer->largestCol*viewer->cellSize + j*viewer->cellSize;
      SDL_RenderFillRect(renderer, &cell);
      if (
        *mouseX/scaleX >= posX + viewer->largestRow*viewer->cellSize + i*viewer->cellSize &&
        *mouseX/scaleX < posX + viewer->largestRow*viewer->cellSize + (i+1)*viewer->cellSize &&
        *mouseY/scaleY >= posY + viewer->largestCol*viewer->cellSize + j*viewer->cellSize &&
        *mouseY/scaleY < posY + viewer->largestCol*viewer->cellSize + (j+1)*viewer->cellSize
      ) {
        cBuffer = viewer->cursorCell(cells[i][j] == 1 ? viewer->cellColor : viewer->blankColor, 35);
        SDL_SetRenderDrawColor(renderer, cBuffer->r, cBuffer->g, cBuffer->b, cBuffer->a);
        SDL_RenderFillRect(renderer, &cell);
      }
    }
  }

  for(int i = 0; i < baseLines.size(); i++) {
    drawLine(
      renderer, baseLines[i][0], baseLines[i][1], baseLines[i][2], baseLines[i][3], viewer->lineWeight, viewer->lineColor
    );
  }
  for(int i = 0; i < darkLines.size(); i++) {
    drawLine(
      renderer, darkLines[i][0], darkLines[i][1], darkLines[i][2], darkLines[i][3], viewer->clusterWeight, viewer->clusterColor
    );
  }


  //Render border lines
  std::vector<SDL_FPoint> points =
  {
    SDL_FPoint{
      (float)(posX + viewer->largestRow*viewer->cellSize),
      (float)(posY + viewer->largestCol*viewer->cellSize)
    },
    SDL_FPoint{
      (float)(posX + viewer->largestRow*viewer->cellSize + cells.size()*viewer->cellSize),
      (float)(posY + viewer->largestCol*viewer->cellSize)
    },
    SDL_FPoint{
      (float)(posX + viewer->largestRow*viewer->cellSize + cells.size()*viewer->cellSize),
      (float)(posY + viewer->largestCol*viewer->cellSize + cells[0].size()*viewer->cellSize)
    },
    SDL_FPoint{
      (float)(posX + viewer->largestRow*viewer->cellSize),
      (float)(posY + viewer->largestCol*viewer->cellSize + cells[0].size()*viewer->cellSize)
    },
    SDL_FPoint{
      (float)(posX + viewer->largestRow*viewer->cellSize),
      (float)(posY + viewer->largestCol*viewer->cellSize)
    },
  };

  traceShape(renderer, points.data(), points.size(), viewer->outlineWeight, viewer->borderColor);

  font->drawScaled(
    renderer,
    scaleX,
    32,
    16,
    48,
    name,
    viewer->borderColor,
    FONT_ALIGN_LEFT
  );

  char dimensions[256];
  std::strcpy(dimensions,const_cast<char*>((std::to_string(cells.size())).c_str())); // copy string one into the result.
  std::strcat(dimensions," x ");
  std::strcat(dimensions,const_cast<char*>((std::to_string(cells[0].size())).c_str()));

  font->drawScaled(
    renderer,
    scaleX,
    32,
    58,
    24,
    dimensions,
    viewer->borderColor,
    FONT_ALIGN_LEFT
  );
}
