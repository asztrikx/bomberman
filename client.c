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