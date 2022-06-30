#ifndef ImageIO_hpp
#define ImageIO_hpp

#include "SDL2/SDL.h"

void save_texture(const char* file_name, SDL_Renderer* renderer, SDL_Texture* texture);

#endif /* ImageIO_hpp */
