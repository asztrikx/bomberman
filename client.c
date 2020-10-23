
#include "client.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdlib.h>
#include "SDL.h"
#include "state.h"
#include "geometry.h"
#include "network.h"

static World* world;
static User* user;
//static long long tickCount = 0; //should be fine for years?
static unsigned int tickRate = 60u;

//ClientSend sends updates to server
Uint32 ClientSend(Uint32 interval, void *param){
	networkSendServer(user);

	return interval;
}

void ClientEventKey(SDL_Event sdl_event){
	//key list update
	KeyItem* current = user->keyItemS;
	while(current != NULL){
		if(current->key == sdl_event.key.keysym.sym){
			//key already in list
			if(sdl_event.type == SDL_KEYDOWN){
				return;
			}

			//key in list, remove
			if(sdl_event.type == SDL_KEYUP){
				if(current->prev == NULL){ //first in list
					user->keyItemS = current->next;

					//last in list also
					if(current->next != NULL){
						current->next->prev = NULL;
					}
				} else if (current->next == NULL) { //last in list, but not first
					current->prev->next = NULL;
				} else {
					current->prev->next = current->next;
					current->next->prev = current->prev;
				}

				//free
				free(current);

				return;
			}
		}

		current = current->next;
	}

	//key not in list, add
	if(sdl_event.type == SDL_KEYDOWN){
		KeyItem* keyItem = (KeyItem*) malloc(sizeof(KeyItem));
		keyItem->key = sdl_event.key.keysym.sym;

		keyItem->next = user->keyItemS;
		keyItem->prev = NULL;

		if(user->keyItemS != NULL){
			user->keyItemS->prev = keyItem;
		}
		
		user->keyItemS = keyItem;
	}
}

//ClientConnect connects to a server
void ClientConnect(void){
	user = (User*) malloc(sizeof(User));
	user->ablityS = NULL;
	user->auth = NULL;
	user->character = NULL;
	user->keyItemS = NULL;
	user->name = "asd";
	user = networkConnectServer(user);

	//server updater
	SDL_AddTimer(1000u/tickRate, ClientSend, NULL);
}

void ClientDraw(void){
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

		objectItemCurrent = objectItemCurrent->next;
	}

	//character
	CharacterItem* characterItemCurrent = world->characterItemS;
	while(characterItemCurrent != NULL){
		if (SDL_SetRenderDrawColor(SDLRenderer, 0, 0, 255, 255) < 0) {
			SDL_Log("SDL_SetRenderDrawColor: %s", SDL_GetError());
			exit(1);
		}

		if(SDL_RenderFillRect(SDLRenderer, &(SDL_Rect){
			.y = characterItemCurrent->character.position.y,
			.x = characterItemCurrent->character.position.x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("SDL_RenderFillRect: %s", SDL_GetError());
			exit(1);
		}

		characterItemCurrent = characterItemCurrent->next;
	}

	SDL_RenderPresent(SDLRenderer);
}

//timeout tick overflow
//ClientReceive gets updates from server
void ClientReceive(World* _world){
	world = _world;

	ClientDraw();
}

void ClientStart(void){
	ClientConnect();
}
