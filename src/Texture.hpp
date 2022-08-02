#ifndef Texture_hpp
#define Texture_hpp

#include <SDL2/SDL.h>

class Texture {
public:
  Texture();
  ~Texture();

  bool fromFile(const char *path);
  void free();
  void createContext(SDL_Renderer *r) {renderer = r;}
  bool createBlank(int width, int height, SDL_TextureAccess = SDL_TEXTUREACCESS_STREAMING );
  void setColor( Uint8 red, Uint8 green, Uint8 blue );
  void setBlendMode( SDL_BlendMode blending );
  void setAlpha( Uint8 alpha );
  void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);
  int getWidth();
  int getHeight();
  void setAsRenderTarget();
  bool lockTexture();
  bool unlockTexture();
  void* getPixels();
  void copyPixels( void* pixels );
  void saveToBMP(const char *filename);
  int getPitch();
  void scale(float s) {mScale = s;}
  Uint32 getPixel32( unsigned int x, unsigned int y );
private:
  SDL_Renderer *renderer;
  SDL_Texture* mTexture;
  SDL_Point origin;
  void* mPixels;
  int mPitch;
  int mWidth;
  int mHeight;
  float mScale = 1;
};

#endif // Texture_hpp
