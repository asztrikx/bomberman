#ifndef SDL_H_INCLUDED
#define SDL_H_INCLUDED

#include <SDL2/SDL.h>

extern SDL_Window* SDLWindow;
extern SDL_Renderer* SDLRenderer;
extern int SDLWindowHeight;
extern int SDLWindowWidth;
void SDLInit(void);

#endif
