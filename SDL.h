#ifndef SDL_H_INCLUDED
#define SDL_H_INCLUDED

#include <SDL2/SDL.h>
#include "type/array.h"

extern SDL_Window* SDLWindow;
extern SDL_Renderer* SDLRenderer;
extern Array** TextureSSObject;
extern int TextureSSLengthObject;
extern Array** TextureSSCharacter;
extern int TextureSSLengthCharacter;
extern int SDLWindowHeight;
extern int SDLWindowWidth;
void SDLInit(void);
void SDLDestroy(void);

#endif
