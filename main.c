#include <SDL2/SDL.h>
#include "SDL.h"
#include "client.h"

int main(void) {
	SDLInit();
	ClientReceive(); //sdl net
	SDL_AddTimer(1000u/60u, ClientDraw, SDLWindow);

	//draw until close
	SDL_RenderPresent(SDLRenderer);
	SDL_Event event;
	while (SDL_WaitEvent(&event) && event.type != SDL_QUIT) {
		switch( event.type ){
            /* Look for a keypress */
            case SDL_KEYDOWN:
                /* Check the SDLKey values and move change the coords */
                switch( event.key.keysym.sym ){
                    case SDLK_LEFT:
                        alien_x -= 1;
                        break;
                    case SDLK_RIGHT:
                        alien_x += 1;
                        break;
                    case SDLK_UP:
                        alien_y -= 1;
                        break;
                    case SDLK_DOWN:
                        alien_y += 1;
                        break;
                    default:
                        break;
                }
            }
        }
	}

	SDL_Quit();
	return 0;
}
