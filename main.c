#include <SDL2/SDL.h>
#include "SDL.h"
#include "client.h"
#include "server.h"

int main(void) {
	srand(time(0)); //better seed?

	ServerStart();

	SDLInit();
	ClientStart();

	SDL_Quit();
	return 0;
}
