#include "geometry.h"
#include "SDL.h"
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "state.h"
#include "network.h"

World* world;
User user;
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

	//exit
	if(world->exit != NULL){
		if (SDL_SetRenderDrawColor(SDLRenderer, 255, 255, 0, 255) < 0) {
			SDL_Log("SDL_SetRenderDrawColor: %s", SDL_GetError());
			exit(1);
		}

		if(SDL_RenderFillRect(SDLRenderer, &(SDL_Rect){
			.y = world->exit->y,
			.x = world->exit->x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("SDL_RenderFillRect: %s", SDL_GetError());
			exit(1);
		}
	}

	//object
	ObjectItem* objectItemCurrent = world->objectItemS;
	while(objectItemCurrent != NULL){
		if (SDL_SetRenderDrawColor(SDLRenderer, 255, 0, 0, 255) < 0) {
			SDL_Log("SDL_SetRenderDrawColor: %s", SDL_GetError());
			exit(1);
		}

		if(SDL_RenderFillRect(SDLRenderer, &(SDL_Rect){
			.y = objectItemCurrent->object->position.y,
			.x = objectItemCurrent->object->position.x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("SDL_RenderFillRect: %s", SDL_GetError());
			exit(1);
		}
	}

	//character
	CharacterItem* characterItemCurrent = world->characterItemS;
	while(characterItemCurrent != NULL){
		if (SDL_SetRenderDrawColor(SDLRenderer, 0, 0, 255, 255) < 0) {
			SDL_Log("SDL_SetRenderDrawColor: %s", SDL_GetError());
			exit(1);
		}

		if(SDL_RenderFillRect(SDLRenderer, &(SDL_Rect){
			.y = characterItemCurrent->character->position.y,
			.x = characterItemCurrent->character->position.x,
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
//ClientReceive gets updates from server
void ClientReceive(World* _world){
	world = _world;
}

//ClientSend sends updates to server
Uint32 ClientSend(Uint32 interval, void *param){
	networkSendServer(user);

	return interval;
}

void ClientEventKey(SDL_Event sdl_event){
	//key list update
	KeyItem* current = user.keyItemS;
	KeyItem* prev = NULL;
	while(current != NULL){
		if(current->key == sdl_event.key.keysym.sym){
			//key already in list
			if(sdl_event.type == SDL_KEYDOWN){
				return;
			}

			//key in list, remove
			if(sdl_event.type == SDL_KEYUP){
				if(prev == NULL){
					//relink
					user.keyItemS = current->next;
					current->next->prev = NULL;
				} else {
					//relink
					prev->next = current->next;
					current->next->prev = prev;
				}

				//free
				free(current);
			}
		}

		prev = current;
		current = current->next;
	}

	//key not in list, add
	if(sdl_event.type == SDL_KEYDOWN){
		KeyItem* keyItem = (KeyItem*) malloc(sizeof(KeyItem));
		keyItem->next = user.keyItemS;
		keyItem->prev = NULL;

		user.keyItemS->prev = keyItem;

		user.keyItemS = keyItem;
	}
}

void ClientStart(void){
	user.name = "teszt";
	user.keyItemS = NULL;

	ClientReceive(); //combined timer with draw
	SDL_AddTimer(1000u/60u, ClientDraw, NULL);
	SDL_AddTimer(1000u/60u, ClientSend, NULL);

	//events
	SDL_Event sdl_event;
	while (SDL_WaitEvent(&sdl_event) && sdl_event.type != SDL_QUIT) {
		if(sdl_event.type == SDL_KEYDOWN || sdl_event.type == SDL_KEYUP){
			ClientEventKey(sdl_event);
		}
	}
}
