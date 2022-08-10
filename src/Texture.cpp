#include "Texture.hpp"
#include "Console.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

Texture::Texture()
{

}

Texture::~Texture()
{

}

int Texture::getWidth()
{
  return mWidth;
}
int Texture::getHeight()
{
  return mHeight;
}

bool Texture::fromFile(const char *path)
{
  std::cout << path << std::endl;
  //Get rid of preexisting texture
  free();

  //The final texture
  SDL_Texture* newTexture = NULL;

  //Load image at specified path
  SDL_Surface* loadedSurface = IMG_Load( path );
  if( loadedSurface == NULL )
  {
    printf( "Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError() );
  }
  else
  {
    //Convert surface to display format
    SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat( loadedSurface, SDL_PIXELFORMAT_RGBA8888, 0 );
    if( formattedSurface == NULL )
    {
      printf( "Unable to convert loaded surface to display format! %s\n", SDL_GetError() );
    }
    else
    {
      //Create blank streamable texture
      newTexture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, formattedSurface->w, formattedSurface->h );
      if( newTexture == NULL )
      {
        printf( "Unable to create blank texture! SDL Error: %s\n", SDL_GetError() );
      }
      else
      {
        //Enable blending on texture
        SDL_SetTextureBlendMode( newTexture, SDL_BLENDMODE_BLEND );

        //Lock texture for manipulation
        SDL_LockTexture( newTexture, &formattedSurface->clip_rect, &mPixels, &mPitch );

        //Copy loaded/formatted surface pixels
        memcpy( mPixels, formattedSurface->pixels, formattedSurface->pitch * formattedSurface->h );

        //Get image dimensions
        mWidth = formattedSurface->w;
        mHeight = formattedSurface->h;

        //Get pixel data in editable format
        Uint32* pixels = (Uint32*)mPixels;
        int pixelCount = ( mPitch / 4 ) * mHeight;

        //Map colors
        Uint32 colorKey = SDL_MapRGB( formattedSurface->format, 0, 0xFF, 0xFF );
        Uint32 transparent = SDL_MapRGBA( formattedSurface->format, 0x00, 0xFF, 0xFF, 0x00 );

        //Color key pixels
        for( int i = 0; i < pixelCount; ++i )
        {
          if( pixels[ i ] == colorKey )
          {
            pixels[ i ] = transparent;
          }
        }

        //Unlock texture to update
        SDL_UnlockTexture( newTexture );
        mPixels = NULL;
      }

      //Get rid of old formatted surface
      SDL_FreeSurface( formattedSurface );
    }

    //Get rid of old loaded surface
    SDL_FreeSurface( loadedSurface );
  }

  //Return success
  mTexture = newTexture;
  return mTexture != NULL;
}

void Texture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
  SDL_Rect renderQuad = SDL_Rect{
    x,y,(int)(mWidth*mScale),(int)(mHeight*mScale)
  };
  SDL_RenderCopy(renderer, mTexture, NULL, &renderQuad);
}

bool Texture::createBlank( int width, int height, SDL_TextureAccess access )
{
  mTexture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA8888, access, width, height );
  if( mTexture == NULL )
  {
    error(SDL_GetError());
  }
  else
  {
    mWidth = width;
    mHeight = height;
  }

  return mTexture != NULL;
}

void Texture::saveToPNG(const char *filename)
{
  SDL_Texture* target = SDL_GetRenderTarget(renderer);
  SDL_SetRenderTarget(renderer, mTexture);
  int width, height;
  SDL_Surface* surface = SDL_CreateRGBSurface(0, mWidth, mHeight, 32, 0, 0, 0, 0);
  SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);
  IMG_SavePNG(surface, filename);
  SDL_FreeSurface(surface);
  SDL_SetRenderTarget(renderer, target);
}

void Texture::setAsRenderTarget()
{
  SDL_SetRenderTarget( renderer, mTexture );
}

bool Texture::lockTexture()
{
  bool success = true;

  //Texture is already locked
  if( mPixels != NULL )
  {
    printf( "Texture is already locked!\n" );
    success = false;
  }
  //Lock texture
  else
  {
    if( SDL_LockTexture( mTexture, NULL, &mPixels, &mPitch ) != 0 )
    {
      printf( "Unable to lock texture! %s\n", SDL_GetError() );
      success = false;
    }
  }

  return success;
}

bool Texture::unlockTexture()
{
  bool success = true;

  //Texture is not locked
  if( mPixels == NULL )
  {
    printf( "Texture is not locked!\n" );
    success = false;
  }
  //Unlock texture
  else
  {
    SDL_UnlockTexture( mTexture );
    mPixels = NULL;
    mPitch = 0;
  }

  return success;
}

void Texture::free()
{
  mTexture = nullptr;
}

Uint32 Texture::getPixel32( unsigned int x, unsigned int y )
{
  //Convert the pixels to 32 bit
  Uint32 *pixels = (Uint32*)mPixels;
  //Get the pixel requested
  return pixels[ ( y * ( mPitch / 4 ) ) + x ];
}
