#include "WinDPI.hpp"
#include "SDL2/SDL.h"

#include <windows.h>
#include <winuser.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#define PROCESS_DPI_UNAWARE 0
#define PROCESS_SYSTEM_DPI_AWARE 1
#define PROCESS_PER_MONITOR_DPI_AWARE 2
#endif

void MySDL_GetDisplayDPI(int displayIndex, float* dpi, float* defaultDpi)
{
  const float kSysDefaultDpi =
  #ifdef __APPLE__
  72.0f;
  #elif defined(_WIN32)
  96.0f;
  #else
  static_assert(false, "No system default DPI set for this platform.");
  #endif

  if (SDL_GetDisplayDPI(displayIndex, NULL, dpi, NULL) != 0)
  {
    // Failed to get DPI, so just return the default value.
    if (dpi) *dpi = kSysDefaultDpi;
  }

  if (defaultDpi) *defaultDpi = kSysDefaultDpi;
}
