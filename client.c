#include "object.h"
#include "character.h"
#include "geometry.h"
#include "SDL.h"
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

Object* objectS;
int objectSLength = 0;
Character* characterS;
int characterSLength = 0;
long long tick = 0; //should be fine for years?

//maybe this header signature should be created main for a wrapper for void ClientDraw
Uint32 ClientDraw(Uint32 interval, void *param){
	//clear & background
	if(SDL_SetRenderDrawColor(SDLRenderer, 0, 255, 0, 255) < 0){
		SDL_Log("SDL_SetRenderDrawColor: %s", SDL_GetError());
		exit(1);
	}
	if(SDL_RenderClear(SDLRenderer) < 0){
		SDL_Log("SDL_RenderClear: %s", SDL_GetError());
		exit(1);
	}

	//object
	for(int i=0; i<objectSLength; i++){
		if (SDL_SetRenderDrawColor(SDLRenderer, 255, 0, 0, 255) < 0) {
			SDL_Log("SDL_SetRenderDrawColor: %s", SDL_GetError());
			exit(1);
		}

		if(SDL_RenderFillRect(SDLRenderer, &(SDL_Rect){
			.y = objectS[i].position.y,
			.x = objectS[i].position.x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("SDL_RenderFillRect: %s", SDL_GetError());
			exit(1);
		}
	}

	//character
	for(int i=0; i<characterSLength; i++){
		if (SDL_SetRenderDrawColor(SDLRenderer, 0, 0, 255, 255) < 0) {
			SDL_Log("SDL_SetRenderDrawColor: %s", SDL_GetError());
			exit(1);
		}

		if(SDL_RenderFillRect(SDLRenderer, &(SDL_Rect){
			.y = characterS[i].position.y,
			.x = characterS[i].position.x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("SDL_RenderFillRect: %s", SDL_GetError());
			exit(1);
		}
	}

	SDL_RenderPresent(SDLRenderer);

	return interval;
}

//timeout tick overflow
void ClientReceive(void){
	objectS = (Object*) malloc(10 * sizeof(Object));
	objectS[0] = (Object){
		.type = ObjectTypeWall,
		.position = (Position){
			.y = 0,
			.x = 0,
		},
	};
	objectSLength = 1;

	characterS = (Character*) malloc(10 * sizeof(Character));
	characterS[0] = (Character){
		.position = (Position){
			.y = 100,
			.x = 100,
		},
		.type = CharacterTypeUser,
		.ablityS = NULL,
	};
	characterSLength = 1;
}

void ClientStart(void){
	ClientReceive();
	SDL_AddTimer(1000u/60u, ClientDraw, SDLWindow);

	//events
	SDL_Event event;
	while (SDL_WaitEvent(&event) && event.type != SDL_QUIT) {
		int deleteme = 10;

		//key
		if(event.type == SDL_KEYDOWN){
			switch(event.key.keysym.sym){
				case SDLK_w:
					characterS[0].position.y -= deleteme;
					break;
				case SDLK_a:
					characterS[0].position.x -= deleteme;
					break;
				case SDLK_s:
					characterS[0].position.y += deleteme;
					break;
				case SDLK_d:
					characterS[0].position.x += deleteme;
					break;
			}
		}
	}
}
