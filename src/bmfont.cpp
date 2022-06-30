#include "bmfont.hpp"
#include "Console.hpp"
#include "stringutil.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <algorithm>

SDL_Texture *pageBuffer[BFONT_PAGE_BUFFER_SIZE];
int pageCount = 0;

std::string fontDir;

bool handleLine(SDL_Renderer *renderer, std::vector<std::string> line, SDL_Texture *pages[], std::map<int, BitmapFont::Glyph> &glyphs, std::map<int, std::map<int,int>> &kerning)
{
  if (line[0].compare("page") == 0) {
    int id;
    std::string path;
    for(std::string s : line) {
      if (s.find("id=") != std::string::npos) {
        id = std::stoi(s.substr(s.find("=")+1));
        pageCount++;
        break;
      }
    }
    for(std::string s : line) {
      if (s.find("file=") != std::string::npos) {
        path = s.substr(s.find("=")+1);
        break;
      }
    }
    if (id >= BFONT_PAGE_BUFFER_SIZE) {
      error("BMFONT_PAGE_BUFFER_SIZE exceeded");
      return false;
    }
    path.erase(std::remove(path.begin(), path.end(), char(34)), path.end());
    fontDir.append(path);
    log(fontDir);
    SDL_Texture *tex = IMG_LoadTexture(renderer, fontDir.c_str());
    if (!tex) {
      error(SDL_GetError());
      return false;
    }
    *(pages + id) = tex;
  }
  else if (line[0].compare("char") == 0) {
    int id;
    for(std::string s : line) {
      if (s.find("id=") != std::string::npos) {
        id = std::stoi(s.substr(s.find("=")+1));
        break;
      }
    }
    for(std::string s : line) {
      if (s.find("x=") != std::string::npos) {
        glyphs[id].x = std::stoi(s.substr(s.find("=")+1));
        continue;
      }
      if (s.find("y=") != std::string::npos) {
        glyphs[id].y = std::stoi(s.substr(s.find("=")+1));
        continue;
      }
      if (s.find("width=") != std::string::npos) {
        glyphs[id].w = std::stoi(s.substr(s.find("=")+1));
        continue;
      }
      if (s.find("height=") != std::string::npos) {
        glyphs[id].h = std::stoi(s.substr(s.find("=")+1));
        continue;
      }
      if (s.find("xoffset=") != std::string::npos) {
        glyphs[id].xOffset = std::stoi(s.substr(s.find("=")+1));
        continue;
      }
      if (s.find("yoffset=") != std::string::npos) {
        glyphs[id].yOffset = std::stoi(s.substr(s.find("=")+1));
        continue;
      }
      if (s.find("xadvance=") != std::string::npos) {
        glyphs[id].xAdvance = std::stoi(s.substr(s.find("=")+1));
        continue;
      }
    }
  }
  else if (line[0].compare("kerning") == 0) {
    int first;
    int second;
    int amount;
    for(std::string s : line) {
      if (s.find("first=") != std::string::npos) {
        first = std::stoi(s.substr(s.find("=")+1));
        continue;
      }
      if (s.find("second=") != std::string::npos) {
        second = std::stoi(s.substr(s.find("=")+1));
        continue;
      }
      if (s.find("amount=") != std::string::npos) {
        amount = std::stoi(s.substr(s.find("=")+1));
        continue;
      }
    }
    kerning[first][second] = amount;
  }
  return true;
}

BitmapFont::BitmapFont()
{

}


BitmapFont::~BitmapFont()
{

}

void BitmapFont::init(SDL_Renderer *renderer, const char *file)
{
  BitmapFont::readAtlas(renderer, file);
}

void BitmapFont::readAtlas(SDL_Renderer *renderer, const char* path)
{
  fontDir = pathDir(std::string(path));
  std::vector<std::string> buffer;
  std::ifstream input(path);
  for( std::string eachLine; getline( input, eachLine ); )
  {
    buffer.clear();
    std::istringstream strm(eachLine);
    std::string splittedLines;
    while ( strm >> splittedLines )
    {
      std::stringstream geek(splittedLines);
      std::string info = geek.str();
      buffer.push_back(info);
    }
    if (!handleLine(renderer, buffer, pages, glyphs, kerning)) return;
  }
  log("Font Loaded!");
}

void BitmapFont::draw(SDL_Renderer *r, float x, float y, float size, const char* text, SDL_Color color, int alignment)
{
  SDL_Rect renderQuad;
  SDL_Rect glyph;
  int xPos = x;
  int lastChar = 0;

  for (int i = 0; i < pageCount; i++) SDL_SetTextureColorMod( *(pages+i), color.r, color.g, color.b );

  int width = 0;
  for (const char *c = text; *c != '\0'; c++) {
    width += (size/glyphs[(int)*c].h)*glyphs[(int)*c].xAdvance + glyphs[(int)*c].xOffset*size/glyphs[(int)*c].h + kerning[lastChar][(int)*c]*size/glyphs[(int)*c].h;
  }

  lastChar = 0;

  if (alignment == FONT_ALIGN_CENTER) xPos = x -width/2;

  for (const char *c = text; *c != '\0'; c++) {

    glyph.x = glyphs[(int)*c].x;
    glyph.y = glyphs[(int)*c].y;
    glyph.w = glyphs[(int)*c].w;
    glyph.h = glyphs[(int)*c].h;

    renderQuad.x = xPos+glyphs[(int)*c].xOffset*size/glyph.h + kerning[lastChar][(int)*c]*size/glyph.h;
    renderQuad.y = y+glyphs[(int)*c].yOffset*size/glyph.h;
    renderQuad.w = size/glyph.h*glyph.w;
    renderQuad.h = size;

    xPos += glyphs[(int)*c].xAdvance*size/glyph.h;
    lastChar = (int)*c;

    if (SDL_RenderCopy( r, *(pages+glyphs[(int)*c].page), &glyph, &renderQuad ) != 0) {
      error(SDL_GetError());
    }
  }
}

void BitmapFont::drawScaled(SDL_Renderer *r, float scaler, float x, float y, float size, const char* text, SDL_Color color, int alignment)
{
  SDL_Rect renderQuad;
  SDL_Rect glyph;
  int xPos = x/scaler;
  int lastChar = 0;

  for (int i = 0; i < pageCount; i++) SDL_SetTextureColorMod( *(pages+i), color.r, color.g, color.b );

  int width = 0;
  for (const char *c = text; *c != '\0'; c++) {
    width += (size/glyphs[(int)*c].h)/scaler * ((glyphs[(int)*c].xAdvance*size/glyphs[(int)*c].h)/scaler) + (glyphs[(int)*c].xOffset*size/glyphs[(int)*c].h)/scaler;
  }

  if (alignment == FONT_ALIGN_CENTER) xPos = (x -width/2)/scaler;

  for (const char *c = text; *c != '\0'; c++) {

    glyph.x = glyphs[(int)*c].x;
    glyph.y = glyphs[(int)*c].y;
    glyph.w = glyphs[(int)*c].w;
    glyph.h = glyphs[(int)*c].h;

    renderQuad.x = xPos+(glyphs[(int)*c].xOffset*size/glyph.h)/scaler + (kerning[lastChar][(int)*c]*size/glyph.h)/scaler;
    renderQuad.y = (y+glyphs[(int)*c].yOffset)/scaler;
    renderQuad.w = (size/glyph.h*glyph.w)/scaler;
    renderQuad.h = size/scaler;

    xPos += (glyphs[(int)*c].xAdvance*size/glyph.h)/scaler;
    lastChar = (int)*c;

    if (SDL_RenderCopy( r, *(pages+glyphs[(int)*c].page), &glyph, &renderQuad ) != 0) {
      error(SDL_GetError());
    }
  }
}
