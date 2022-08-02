#include "Nonogram.hpp"
#include "Console.hpp"
#include "GFX.hpp"
#include "WinDPI.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include <cmath>
#include <cstdio>

#include <stdlib.h>
#include <stdio.h>
#include <fstream>

#include "nfd.hpp"
#include "Texture.hpp"
#include "Solver.hpp"
#include "ColorUtil.hpp"

#ifdef __cplusplus
extern "C" {
#endif
    #include "osdialog.h"
#ifdef __cplusplus
}
#endif

typedef std::vector<int> tRow;

Nonogram::Nonogram()
{

}

MouseObject::MouseObject()
{

}

Nonogram::~Nonogram()
{

}

void Nonogram::update()
{
  int mX, mY;
  Nonogram::scaledMousePos(&mX, &mY);
  int hintRowOffset = viewer->showHints ? viewer->largestRow*viewer->cellSize : 0;
  int hintColOffset = viewer->showHints ? viewer->largestCol*viewer->cellSize : 0;
  if (
    mX >= X + hintRowOffset*(*zoom) &&
    mY >= Y + hintColOffset*(*zoom)
  ){
    int qX, qY;
    Nonogram::cellPos(&qX,&qY);
    if (qX < cells.size() && qY < cells[0].size()) {
      int newCell = currentButton == SDL_BUTTON_LEFT ? 1 : 0;
      if (cells[qX][qY] != newCell && currentButton >= 0){
        cells[qX][qY] = newCell;
        Nonogram::refresh();
      }
    }
  } else if (!Nonogram::contains(mX, mY) && currentButton >= 0) {
    currentButton = -1;
  }
}

void Nonogram::onClick(SDL_Event e) {
  currentButton = e.button.button;
}

void Nonogram::onRelease(SDL_Event e){
  if (currentButton == e.button.button) currentButton = -1;
}

void Nonogram::savePuzzle()
{
  Texture *t = generateBitmap();
  const char* filename = (std::string(name) + ".hbm").c_str();
  nfdchar_t* savePath;
  // prepare filters for the dialog
  nfdfilteritem_t filterItem[2] = {{"Hanjie Bitmap", "hbm, bmp"}};

  // show the dialog
  nfdresult_t result = NFD_SaveDialog(&savePath, filterItem, 1, NULL, filename);
  if (result == NFD_OKAY) {
      puts("Success!");
      puts(savePath);
      t->saveToBMP(savePath);
      NFD_FreePath(savePath);
  } else if (result == NFD_CANCEL) {
      puts("User pressed cancel.");
  } else {
      printf("Error: %s\n", NFD_GetError());
  }
}

void Nonogram::savePuzzleTxt()
{
  const char* filename = (std::string(name) + ".txt").c_str();
  nfdchar_t* savePath;
  // prepare filters for the dialog
  nfdfilteritem_t filterItem[2] = {{"Text File", "txt"}};
  // show the dialog
  nfdresult_t result = NFD_SaveDialog(&savePath, filterItem, 1, NULL, filename);
  if (result == NFD_OKAY) {
      puts("Success!");
      puts(savePath);
      std::ofstream MyFile(savePath);

      // Write to the file
      MyFile << "{ver:";
      MyFile << "[";
      for (std::vector<int> row : rows) {
        MyFile << "[";
        for (size_t i = 0; i < row.size(); i++) {
          MyFile << (i > 0 ? "," : "") << row[i];
        }
        MyFile << "],";
      }
      MyFile << "],";
      MyFile << "[";
      for (std::vector<int> col : columns) {
        MyFile << "[";
        for (size_t i = 0; i < col.size(); i++) {
          MyFile << (i > 0 ? "," : "") << col[i];
        }
        MyFile << "],";
      }
      MyFile << "]}";
      MyFile.close();
      NFD_FreePath(savePath);
  } else if (result == NFD_CANCEL) {
      puts("User pressed cancel.");
  } else {
      printf("Error: %s\n", NFD_GetError());
  }
}

void Nonogram::loadPuzzle()
{
  nfdchar_t* outPath;
  nfdfilteritem_t filterItem[2] = {{"Hanjie Bitmap", "hbm, bmp"}};

  // show the dialog
  nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 2, NULL);
  if (result == NFD_OKAY) {
      puts("Success!");
      puts(outPath);

      Texture *tex = new Texture();
      tex->createContext(renderer);
      if (tex->fromFile(outPath)){
        cells = fromTexture(tex);

        //Initialize hint rows and columns
        rows = std::vector<std::vector<int>>(tex->getHeight(), std::vector<int> (1, 0));
        columns = std::vector<std::vector<int>>(tex->getWidth(), std::vector<int> (1, 0));

        refresh();
      } else {
        error("Error loading puzzle");
      }
      // remember to free the memory (since NFD_OKAY is returned)
      NFD_FreePath(outPath);
  } else if (result == NFD_CANCEL) {
      puts("User pressed cancel.");
  } else {
      printf("Error: %s\n", NFD_GetError());
  }
}

Texture *Nonogram::generateBitmap()
{
  float scaleX, scaleY;
  SDL_RenderGetScale(renderer, &scaleX, &scaleY);

  Texture *t = new Texture();
  t->createContext(renderer);
  t->createBlank(cells.size(), cells[0].size(), SDL_TEXTUREACCESS_TARGET);

  t->setAsRenderTarget();
  SDL_RenderClear(renderer);
  SDL_RenderSetScale(renderer,1,1);

  int rgbVals[2] = {255, 0};

  for (size_t i = 0; i < cells.size(); i++) {
    for(size_t j = 0; j < cells[i].size(); j++) {
      SDL_SetRenderDrawColor(
        renderer,
        rgbVals[cells[i][j]],
        rgbVals[cells[i][j]],
        rgbVals[cells[i][j]],
        255
      );
      SDL_RenderDrawPoint(renderer, i, j);
    }
  }
  SDL_Point p = SDL_Point{0,0};
  SDL_SetRenderTarget(renderer,NULL);
  SDL_RenderSetScale(renderer, scaleX,scaleY);
  t->render(0, 0, NULL, 0.0);
  return t;
}

std::vector<std::vector<int>> Nonogram::fromTexture(Texture *img)
{
  img->lockTexture();
  std::vector<std::vector<int>> newCells(img->getWidth(), std::vector<int>(img->getHeight()));

  for (int i = 0; i < img->getWidth(); i++) {
    for(int j = 0; j < img->getHeight(); j++) {
      Uint32 pixel = img->getPixel32(i,j);

      float vR = (float)((pixel >> 16) & 0xff) / 255;
      float vG = (float)((pixel >> 8) & 0xff) / 255;
      float vB = (float)(pixel & 0xff) / 255;

      float Ylum = (0.2126f * sRGBtoLin(vR) + 0.7152f * sRGBtoLin(vG) + 0.0722f * sRGBtoLin(vB));

      float lightness = YtoLstar(Ylum);

      if (lightness < 49)
        newCells[i][j] = 1;
      else
        newCells[i][j] = 0;
    }
  }

  img->unlockTexture();
  return newCells;
}

void Nonogram::cellPos(int *qX, int *qY) {
  float scaleX, scaleY;
  SDL_RenderGetScale(renderer, &scaleX, &scaleY);

  int hintRowOffset = viewer->showHints ? viewer->largestRow*viewer->cellSize : 0;
  int hintColOffset = viewer->showHints ? viewer->largestCol*viewer->cellSize : 0;

  *qX = ((*mouseX/scaleX) - X - hintRowOffset*(*zoom))/(viewer->cellSize*(*zoom));
  *qY = ((*mouseY/scaleY) - Y - hintColOffset*(*zoom))/(viewer->cellSize*(*zoom));
}

void Nonogram::scaledMousePos(int *qX, int *qY) {
  float scaleX, scaleY;
  SDL_RenderGetScale(renderer, &scaleX, &scaleY);

  *qX = ((*mouseX)/scaleX);
  *qY = ((*mouseY)/scaleY);
}

void Nonogram::bindMouseControls() {
  mouseHandler->callbacks[SDL_MOUSEBUTTONDOWN] = [this](SDL_Event e){
    Nonogram::onClick(e);
  };
  mouseHandler->callbacks[SDL_MOUSEBUTTONUP] = [this](SDL_Event e){
    Nonogram::onRelease(e);
  };
  //on release
  //handler->callbacks[SDL_MOUSEBUTTONUP] = [e](){ onRelease(e); }
}

bool Nonogram::contains(int x, int y) {
  return (
    x >= X && x < X+Nonogram::getWidth()*(*zoom) &&
    y >= Y && y < Y+Nonogram::getHeight()*(*zoom)
  );
}

void Nonogram::fitToView()
{
  int vw;
  int vh;
  SDL_GetWindowSize(window, &vw, &vh);

  int w = Nonogram::getWidth();
  int h = Nonogram::getHeight();

  float result = std::min(0.66f*(float)vw/(float)w, 0.8f*(float)vh/(float)h);

  SDL_RenderSetScale(renderer, result, result);
}

void Nonogram::refresh()
{
  //fill column numbers
  for(size_t i = 0; i < cells.size(); i++) {
    std::vector<int> col(0);
    int count = 0;
    for(size_t j = 0; j < cells[i].size(); j++) {
      if (cells[i][j] == 1) count++;
      if (cells[i][j] == 0 || j+1 >= cells[i].size()) {
        if (count > 0 || (j+1 >= cells[i].size() && col.size() == 0)) col.push_back(count);
        count = 0;
      }
    }
    columns[i] = col;
  }

  //fill row numbers
  for(size_t i = 0; i < cells[0].size(); i++) {
    std::vector<int> row(0);
    int count = 0;
    for(size_t j = 0; j < cells.size(); j++) {
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
  for(size_t i = 0; i < rows.size(); i++) {
    if (rows[i].size() > temp) temp = rows[i].size();
  }
  viewer->largestRow = temp;

  temp = 0;
  for(size_t i = 0; i < columns.size(); i++) {
    if (columns[i].size() > temp) temp = columns[i].size();
  }
  viewer->largestCol = temp;
}

void Nonogram::init(const char* title, int sizeX, int sizeY)
{
  viewer = new Viewer();
  viewer->init(48, toggles[VIEWER_HINTS]);
  //Initialize mouse handling structures
  mouseHandler = new MouseObject();
  //Define mouse related behavior specific to this object
  bindMouseControls();

  //Title of nonogram puzzle
  name = title;

  //Initialize puzzle cells
  cells = std::vector<std::vector<int>>(sizeX, std::vector<int> (sizeY, 0));

  //Initialize hint rows and columns
  rows = std::vector<std::vector<int>>(sizeY, std::vector<int> (1, 0));
  columns = std::vector<std::vector<int>>(sizeX, std::vector<int> (1, 0));

  //Initialize solver class
  solver = new Solver();
  solver->init(renderer);

  //Generate fully random cell fills
  Nonogram::randomize();

  //solvePuzzle();
}

void Nonogram::randomize()
{
  for(size_t i = 0; i < cells.size(); i++) {
    for(size_t j = 0; j < cells[i].size(); j++) {
      cells[i][j] = rand()%2 < 1 ? 1 : 0;
    }
  }
  Nonogram::refresh();
}

int Nonogram::getWidth()
{
  return
    cells.size()*viewer->cellSize +
    (viewer->showHints ? viewer->largestRow*viewer->cellSize : 0);
}

int Nonogram::getHeight()
{
  return
    cells[0].size()*viewer->cellSize +
    (viewer->showHints ? viewer->largestCol*viewer->cellSize : 0);
}

void Nonogram::setPosition(int x, int y)
{
  X = x - (viewer->showHints ? (viewer->largestRow*viewer->cellSize)/4 : 0);
  Y = y;
}

void Nonogram::solvePuzzle()
{
  //Initialize solver class
  solver->~Solver();
  solver = new Solver();
  solver->init(renderer);
  paused = true;
  bool valid = solver->solve(this,viewer->cellSize);
  if(valid)
    log("Puzzle is solvable!");
  else
    log("Puzzle is impossible to solve with logic alone... try altering.");
  SDL_Delay(3000);
  paused = false;
}

void Nonogram::render()
{
  //HUD
  float scaleX, scaleY;
  SDL_RenderGetScale(renderer, &scaleX, &scaleY);

  std::vector<SDL_Color> gradientBuffer;
  std::vector<std::vector<int>> baseLines;
  std::vector<std::vector<int>> darkLines;

  int cellSize = (int)((float)viewer->cellSize*(*zoom));

  int hintRowOffset = viewer->showHints ? viewer->largestRow*cellSize : 0;
  int hintColOffset = viewer->showHints ? viewer->largestCol*cellSize : 0;

  //Render row dividers
  for(size_t i = 0; i <= rows.size(); i++) {
    if (i % 5 == 0) {
      darkLines.push_back({
        (int)(X + hintRowOffset),
        (int)(Y + hintColOffset + i*cellSize),
        (int)(X + hintRowOffset + cells.size()*cellSize),
        (int)(Y + hintColOffset + i*cellSize)
      });
    } else {
      baseLines.push_back({
        (int)(X + hintRowOffset),
        (int)(Y + hintColOffset + i*cellSize),
        (int)(X + hintRowOffset + cells.size()*cellSize),
        (int)(Y + hintColOffset + i*cellSize)
      });
    }

    if (i < rows.size()){
      bool highlight = false;
      gradientBuffer.clear();
      if (
        *mouseX/scaleX >= X&&
        *mouseX/scaleX < X + hintRowOffset+ cells.size()*cellSize &&
        *mouseY/scaleY >= Y + hintColOffset + i*cellSize &&
        *mouseY/scaleY < Y + hintColOffset + (i+1)*cellSize
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
        X, Y + hintColOffset + i*cellSize,
        hintRowOffset, cellSize,
        gradientBuffer,
        GRECT_RIGHT
      );

      if (viewer->showHints) for(size_t h = 0; h < rows[i].size(); h++) {
        font->draw(
          renderer,
          X + hintRowOffset - (h+1)*cellSize + cellSize/2,
          Y + hintColOffset + i*cellSize + (cellSize-viewer->fontSize*(*zoom))/2,
          viewer->fontSize*(*zoom),
          const_cast<char*>((std::to_string(rows[i][rows[i].size()-h-1])).c_str()),
          highlight ? viewer->focusColor : viewer->borderColor,
          FONT_ALIGN_CENTER
        );
      }
    }
  }

  //Render column dividers
  for(size_t i = 0; i <= columns.size(); i++) {
    if (i % 5 == 0) {
      darkLines.push_back({
        (int)(X + hintRowOffset + i*cellSize),
        (int)(Y + hintColOffset),
        (int)(X + hintRowOffset + i*cellSize),
        (int)(Y + hintColOffset + cells[0].size()*cellSize)
      });
    } else {
      baseLines.push_back({
        (int)(X + hintRowOffset + i*cellSize),
        (int)(Y + hintColOffset),
        (int)(X + hintRowOffset + i*cellSize),
        (int)(Y + hintColOffset + cells[0].size()*cellSize)
      });
    }

    gradientBuffer.clear();
    gradientBuffer.push_back(SDL_Color{0,0,0,0});
    gradientBuffer.push_back(SDL_Color{0,0,0,50});

    if (i < columns.size()) {
      bool highlight = false;
      gradientBuffer.clear();
      if (
        *mouseY/scaleY >= Y&&
        *mouseY/scaleY < Y + hintColOffset+ cells[0].size()*cellSize &&
        *mouseX/scaleX >= X + hintRowOffset + i*cellSize &&
        *mouseX/scaleX < X + hintRowOffset + (i+1)*cellSize
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
        X + hintRowOffset + i*cellSize, Y,
        cellSize, hintColOffset,
        gradientBuffer,
        GRECT_DOWN
      );

      if (viewer->showHints) for(size_t h = 0; h < columns[i].size(); h++) {
        font->draw(
          renderer,
          X + hintRowOffset + i*cellSize + cellSize/2,
          Y + hintColOffset - (h+1)*cellSize,
          viewer->fontSize*(*zoom),
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
  cell.w = cellSize;
  cell.h = cellSize;

  for(size_t i = 0; i < cells.size(); i++) {
    for(size_t j = 0; j < cells[0].size(); j++) {
      cBuffer = cells[i][j] == 1 ? &viewer->cellColor : &viewer->blankColor;
      SDL_SetRenderDrawColor(renderer, cBuffer->r, cBuffer->g, cBuffer->b, cBuffer->a);
      cell.x = X + hintRowOffset + i*cellSize;
      cell.y = Y + hintColOffset + j*cellSize;
      SDL_RenderFillRect(renderer, &cell);
      if (
        *mouseX/scaleX >= X + hintRowOffset + i*cellSize &&
        *mouseX/scaleX < X + hintRowOffset + (i+1)*cellSize &&
        *mouseY/scaleY >= Y + hintColOffset + j*cellSize &&
        *mouseY/scaleY < Y + hintColOffset + (j+1)*cellSize
      ) {
        cBuffer = viewer->cursorCell(cells[i][j] == 1 ? viewer->cellColor : viewer->blankColor, 35);
        SDL_SetRenderDrawColor(renderer, cBuffer->r, cBuffer->g, cBuffer->b, cBuffer->a);
        SDL_RenderFillRect(renderer, &cell);
      }
    }
  }

  for(size_t i = 0; i < baseLines.size(); i++) {
    drawLine(
      renderer, baseLines[i][0], baseLines[i][1], baseLines[i][2], baseLines[i][3], viewer->lineWeight*(*zoom), viewer->lineColor
    );
  }
  for(size_t i = 0; i < darkLines.size(); i++) {
    drawLine(
      renderer, darkLines[i][0], darkLines[i][1], darkLines[i][2], darkLines[i][3], viewer->clusterWeight*(*zoom), viewer->clusterColor
    );
  }


  //Render border lines
  std::vector<SDL_FPoint> points =
  {
    SDL_FPoint{
      (float)(X + hintRowOffset),
      (float)(Y + hintColOffset)
    },
    SDL_FPoint{
      (float)(X + hintRowOffset + cells.size()*cellSize),
      (float)(Y + hintColOffset)
    },
    SDL_FPoint{
      (float)(X + hintRowOffset + cells.size()*cellSize),
      (float)(Y + hintColOffset + cells[0].size()*cellSize)
    },
    SDL_FPoint{
      (float)(X + hintRowOffset),
      (float)(Y + hintColOffset + cells[0].size()*cellSize)
    },
    SDL_FPoint{
      (float)(X + hintRowOffset),
      (float)(Y + hintColOffset)
    },
  };

  traceShape(renderer, points.data(), points.size(), viewer->outlineWeight*(*zoom), viewer->borderColor);

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

  if (paused) {
    solver->render();
  }
}
