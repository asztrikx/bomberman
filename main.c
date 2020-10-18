#include <SDL2/SDL.h>
#include "SDL.h"
#include "client.h"

int main(void) {
	SDLInit();
	ClientStart();

	SDL_Quit();
	return 0;
}
