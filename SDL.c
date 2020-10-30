#include "debugmalloc.h"
#include "SDL.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "geometry.h"

SDL_Window* SDLWindow;
SDL_Renderer* SDLRenderer;

void SDLInit(void){
	//init
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		SDL_Log("sdl init: %s", SDL_GetError());
		exit(1);
	}

	/*if (TTF_Init() < 0) {
		SDL_Log("sdl init: %s", SDL_GetError());
		exit(1);
	}*/

	//window
	SDLWindow = SDL_CreateWindow("alt", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, 0);
	if (SDLWindow == NULL) {
		SDL_Log("sdl window: %s", SDL_GetError());
		exit(1);
	}

	//render
	SDLRenderer = SDL_CreateRenderer(SDLWindow, -1, SDL_RENDERER_SOFTWARE);
	if (SDLRenderer == NULL) {
		SDL_Log("sdl render: %s", SDL_GetError());
		exit(1);
	}
	SDL_RenderClear(SDLRenderer);
}
