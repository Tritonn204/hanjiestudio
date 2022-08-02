#ifndef ImageIO_hpp
#define ImageIO_hpp

#include "SDL2/SDL.h"

void save_texture(const char* file_name, SDL_Renderer* renderer, SDL_Texture* texture);

void save_texture_bmp(const char *filename, SDL_Renderer *ren, SDL_Texture *tex);

#endif /* ImageIO_hpp */
