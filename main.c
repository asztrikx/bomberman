#include "debugmalloc.h"
#include <SDL2/SDL.h>
#include "SDL.h"
#include "client.h"
#include "server.h"
#include "time.h"
#include <stdbool.h>

int main(int argc, char *argv[]) {
	//secure seed generation
	//-
	srand(time(0));

	SDLInit();

	if(argc >= 2){
		if(strcmp(argv[1], "load") == 0){
			ServerStart(true);
		} else {
			puts("unknown argument");
			exit(1);
		}
	} else {
		ServerStart(false);
	}
	
	ClientStart();
	ClientConnect();

	SDL_Event sdl_event;
	while (SDL_WaitEvent(&sdl_event) && sdl_event.type != SDL_QUIT) {
	}

	ClientStop();
	ServerStop();
	
	SDLDestroy();


	return 0;
}
