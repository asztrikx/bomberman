#include "debugmalloc.h"
#include <SDL2/SDL.h>
#include "SDL.h"
#include "client.h"
#include "server.h"
#include "time.h"

int main(void) {
	srand(time(0)); //[R] better seed

	SDLInit();

	ServerStart();
	ClientStart();
	ClientConnect();

	//key press
	SDL_Event sdl_event;
	while (SDL_WaitEvent(&sdl_event) && sdl_event.type != SDL_QUIT) {
		if(sdl_event.type == SDL_KEYDOWN || sdl_event.type == SDL_KEYUP){
			ClientEventKey(sdl_event);
		}
	}

	ServerStop();
	ClientStop();
	
	SDL_Quit();
	return 0;
}
