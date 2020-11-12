#include "debugmalloc.h"
#include <SDL2/SDL.h>
#include "SDL.h"
#include "client.h"
#include "server.h"
#include "time.h"

int main(int argc, char *argv[]) {
	srand(time(0)); //[R] better seed

	//local
	if(argc < 2){
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

		ClientStop();
		ServerStop();
		
		SDLDestroy();
	} else if(strcmp(argv[1], "server") == 0){
		ServerStart();

		while(getchar() != EOF){
			puts("Send EOF to stop server");
		}

		ServerStop();
	} else if(strcmp(argv[1], "client") == 0){
		SDLInit();

		ClientStart();
		ClientConnect();

		//key press
		SDL_Event sdl_event;
		while (SDL_WaitEvent(&sdl_event) && sdl_event.type != SDL_QUIT) {
			if(sdl_event.type == SDL_KEYDOWN || sdl_event.type == SDL_KEYUP){
				ClientEventKey(sdl_event);
			}
		}

		ClientStop();

		SDLDestroy();
	}

	return 0;
}
