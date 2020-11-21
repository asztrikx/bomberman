#include "debugmalloc.h"
#include <SDL2/SDL.h>
#include "SDL.h"
#include "client.h"
#include "server.h"
#include "time.h"

int main(int argc, char *argv[]) {
	//secure seed generation
	//-
	srand(time(0));

	//local
	if(argc < 2){
		SDLInit();

		ServerStart();
		ClientStart();
		ClientConnect();

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

		ClientStop();

		SDLDestroy();
	}

	return 0;
}
