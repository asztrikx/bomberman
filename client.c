#include "debugmalloc.h"
#include "client.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdlib.h>
#include "SDL.h"
#include "state.h"
#include "geometry.h"
#include "network.h"

static UserClient* userClient;
//static long long tickCount = 0; //should be fine for years?
static unsigned int tickRate = 60u;

//ClientSend sends updates to server
Uint32 ClientSend(Uint32 interval, void *param){
	if(userClient->keyItemS == NULL){
		return interval;
	}
	
	networkSendServer(userClient);

	return interval;
}

void ClientEventKey(SDL_Event sdl_event){
	//key list update
	KeyItem* keyItemCurrent = userClient->keyItemS;
	while(keyItemCurrent != NULL){
		if(keyItemCurrent->key == sdl_event.key.keysym.sym){
			//key already in list
			if(sdl_event.type == SDL_KEYDOWN){
				return;
			}

			//key in list, remove
			if(sdl_event.type == SDL_KEYUP){
				if(keyItemCurrent->prev == NULL){ //first in list
					userClient->keyItemS = keyItemCurrent->next;

					//last in list also
					if(keyItemCurrent->next != NULL){
						keyItemCurrent->next->prev = NULL;
					}
				} else if (keyItemCurrent->next == NULL) { //last in list, but not first
					keyItemCurrent->prev->next = NULL;
				} else {
					keyItemCurrent->prev->next = keyItemCurrent->next;
					keyItemCurrent->next->prev = keyItemCurrent->prev;
				}

				//free
				free(keyItemCurrent);

				return;
			}
		}

		keyItemCurrent = keyItemCurrent->next;
	}

	//key not in list, add
	if(sdl_event.type == SDL_KEYDOWN){
		KeyItem* keyItem = (KeyItem*) malloc(sizeof(KeyItem));
		keyItem->key = sdl_event.key.keysym.sym;

		keyItem->next = userClient->keyItemS;
		keyItem->prev = NULL;

		if(userClient->keyItemS != NULL){
			userClient->keyItemS->prev = keyItem;
		}
		
		userClient->keyItemS = keyItem;
	}
}

//ClientConnect connects to a server
void ClientConnect(void){
	networkConnectServer(userClient);

	//server updater
	SDL_AddTimer(1000u/tickRate, ClientSend, NULL);
}

void ClientDraw(WorldClient* worldClient){
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
	if(worldClient->exit != NULL){
		if (SDL_SetRenderDrawColor(SDLRenderer, 255, 255, 0, 255) < 0) {
			SDL_Log("SDL_SetRenderDrawColor: %s", SDL_GetError());
			exit(1);
		}

		if(SDL_RenderFillRect(SDLRenderer, &(SDL_Rect){
			.y = worldClient->exit->y,
			.x = worldClient->exit->x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("SDL_RenderFillRect: %s", SDL_GetError());
			exit(1);
		}
	}

	//object
	for(int i=0; i<worldClient->objectSLength; i++){
		if (SDL_SetRenderDrawColor(SDLRenderer, 255, 0, 0, 255) < 0) {
			SDL_Log("SDL_SetRenderDrawColor: %s", SDL_GetError());
			exit(1);
		}

		if(SDL_RenderFillRect(SDLRenderer, &(SDL_Rect){
			.y = worldClient->objectS[i].position.y,
			.x = worldClient->objectS[i].position.x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("SDL_RenderFillRect: %s", SDL_GetError());
			exit(1);
		}
	}

	//character
	for(int i=0; i<worldClient->characterSLength; i++){
		if (SDL_SetRenderDrawColor(SDLRenderer, 0, 0, 255, 255) < 0) {
			SDL_Log("SDL_SetRenderDrawColor: %s", SDL_GetError());
			exit(1);
		}

		if(SDL_RenderFillRect(SDLRenderer, &(SDL_Rect){
			.y = worldClient->characterS[i].position.y,
			.x = worldClient->characterS[i].position.x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("SDL_RenderFillRect: %s", SDL_GetError());
			exit(1);
		}
	}

	SDL_RenderPresent(SDLRenderer);
}

//ClientReceive gets updates from server
void ClientReceive(WorldClient* worldCopy){
	ClientDraw(worldCopy);
}

void ClientStart(void){
	//userClient create
	userClient = (UserClient*) malloc(sizeof(UserClient));
	userClient->ablityS = NULL;
	userClient->auth = NULL;
	userClient->keyItemS = NULL;
	userClient->name = (char*) malloc((15 + 1) * sizeof(char));

	//userClient load
	strcpy(userClient->name, "asd");
}
