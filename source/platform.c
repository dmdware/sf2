




#include "platform.h"

#ifdef PLATFORM_WIN
HINSTANCE g_hInstance = NULL;
#endif

SDL_Window *g_win;
SDL_GLContext g_gx;
