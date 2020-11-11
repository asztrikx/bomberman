#include "debugmalloc.h"
#include "client.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include "SDL.h"
#include "type/client/user.h"
#include "type/client/world.h"
#include "type/geometry.h"
#include "config.h"
#include "network.h"
#include "state.h"

static SDL_mutex* mutex;
static UserClient* userClient;
//static long long tickCount = 0; //should be fine for years?
static unsigned int tickRate = 60u;
static int tickId;

//ClientSend sends updates to server
Uint32 ClientSend(Uint32 interval, void *param){
	if (SDL_LockMutex(mutex) != 0){
		SDL_Log("ClientSend: SDL_LockMutex: %s", SDL_GetError());
		exit(1);
	}
	
	networkSendServer(userClient);

	if(SDL_UnlockMutex(mutex) < 0){
		SDL_Log("ClientSend: mutex unlock: %s", SDL_GetError());
		exit(1);
	}

	return interval;
}

void ClientEventKey(SDL_Event sdl_event){
	if (SDL_LockMutex(mutex) != 0){
		SDL_Log("ClientEventKey: SDL_LockMutex: %s", SDL_GetError());
		exit(1);
	}

	//key list update
	bool in = false;
	for(ListItem* item = userClient->keyList->head; item != NULL; item = item->next) {
		if(*(SDL_Keycode*)item->data != sdl_event.key.keysym.sym){
			continue;
		}
			
		//key already in list
		if(sdl_event.type == SDL_KEYDOWN){
			in = true;
			break;
		}

		//key in list, remove
		if(sdl_event.type == SDL_KEYUP){
			ListRemoveItem(&(userClient->keyList), item, intfree);
			break;
		}
	}

	//key not in list, add
	if(sdl_event.type == SDL_KEYDOWN && !in){
		ListInsert(&(userClient->keyList), Copy(&sdl_event.key.keysym.sym, sizeof(SDL_Keycode)));
	}

	if(SDL_UnlockMutex(mutex) < 0){
		SDL_Log("ClientEventKey: mutex unlock: %s", SDL_GetError());
		exit(1);
	}
}

//ClientConnect connects to a server
void ClientConnect(void){
	networkClientStart();

	networkConnectServer(userClient); //not critical section

	//server updater
	tickId = SDL_AddTimer(1000u/tickRate, ClientSend, NULL);
	if (tickId == 0){
		SDL_Log("SDL_AddTimer: %s", SDL_GetError());
		exit(1);
	}
}

void ClientDraw(WorldClient* worldClient){
	//character find
	Character* characterMe = NULL;
	for(int i=0; i<worldClient->characterSLength; i++){
		if(worldClient->characterS[i].type == CharacterTypeYou){
			characterMe = &(worldClient->characterS[i]);
			break;
		}
	}

	//died
	if(characterMe == NULL){
		if(SDL_SetRenderDrawColor(SDLRenderer, 255, 0, 0, 0) < 0){
			SDL_Log("ClientDraw: SDL_SetRenderDrawColor: %s", SDL_GetError());
			exit(1);
		}
		if(SDL_RenderClear(SDLRenderer) < 0){
			SDL_Log("ClientDraw: SDL_RenderClear: %s", SDL_GetError());
			exit(1);
		}

		//render
		SDL_RenderPresent(SDLRenderer);
		return;
	}

	//offset
	Position offset = (Position){
		.y = -characterMe->position.y + windowHeight / 2,
		.x = -characterMe->position.x + windowWidth / 2,
	};

	//clear & background
	if(SDL_SetRenderDrawColor(SDLRenderer, 0, 255, 0, 255) < 0){
		SDL_Log("ClientDraw: SDL_SetRenderDrawColor: %s", SDL_GetError());
		exit(1);
	}
	if(SDL_RenderClear(SDLRenderer) < 0){
		SDL_Log("ClientDraw: SDL_RenderClear: %s", SDL_GetError());
		exit(1);
	}

	//exit
	if(worldClient->exit != NULL){
		if (SDL_SetRenderDrawColor(SDLRenderer, 255, 255, 0, 255) < 0) {
			SDL_Log("ClientDraw: SDL_SetRenderDrawColor: %s", SDL_GetError());
			exit(1);
		}

		if(SDL_RenderFillRect(SDLRenderer, &(SDL_Rect){
			.y = worldClient->exit->y + offset.y,
			.x = worldClient->exit->x + offset.x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("ClientDraw: SDL_RenderFillRect: %s", SDL_GetError());
			exit(1);
		}
	}

	//object
	for(int i=0; i<worldClient->objectSLength; i++){
		int r,g,b;

		switch (worldClient->objectS[i].type){
			case ObjectTypeBomb:
				r = 0;
				g = 0;
				b = 0;
				break;
			case ObjectTypeWall:
				r = 255;
				g = 0;
				b = 0;
				break;
			case ObjectTypeBombFire:
				r = 255;
				g = 165;
				b = 0;
				break;
			case ObjectTypeBox:
				r = 165;
				g = 165;
				b = 42;
				break;
			
			default:
				r = 155;
				g = 155;
				b = 155;
				break;
		}
		if (SDL_SetRenderDrawColor(SDLRenderer, r, g, b, 255) < 0) {
			SDL_Log("ClientDraw: SDL_SetRenderDrawColor: %s", SDL_GetError());
			exit(1);
		}

		if(SDL_RenderFillRect(SDLRenderer, &(SDL_Rect){
			.y = worldClient->objectS[i].position.y + offset.y,
			.x = worldClient->objectS[i].position.x + offset.x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("ClientDraw: SDL_RenderFillRect: %s", SDL_GetError());
			exit(1);
		}
	}

	//character
	for(int i=0; i<worldClient->characterSLength; i++){
		if (SDL_SetRenderDrawColor(SDLRenderer, 0, 0, 255, 255) < 0) {
			SDL_Log("ClientDraw: SDL_SetRenderDrawColor: %s", SDL_GetError());
			exit(1);
		}

		if(SDL_RenderFillRect(SDLRenderer, &(SDL_Rect){
			.y = worldClient->characterS[i].position.y + offset.y,
			.x = worldClient->characterS[i].position.x + offset.x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("ClientDraw: SDL_RenderFillRect: %s", SDL_GetError());
			exit(1);
		}
	}

	//character name
	for(int i=0; i<worldClient->characterSLength; i++){
		/*if (stringRGBA(
			SDLRenderer,
			worldClient->characterS[i].position.x + offset.x,
			worldClient->characterS[i].position.y + 10 + offset.y,
			worldClient->characterS[i].name,
			0, 0, 0, 255
		) > 0){
			SDL_Log("stringRGBA: %s", SDL_GetError());
			exit(1);
		}*/

		/*TTF_Font* font = TTF_OpenFont("NotoSansMono-Regular.ttf", 24); //this opens a font style and sets a size
		if(!font) {
			printf("TTF_OpenFont: %s\n", TTF_GetError());
			exit(1);
		}
		SDL_Color Black = {0, 0, 0};  // this is the color in rgb format, maxing out all would give you the color white, and it will be your text's color
		SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, worldClient->characterS[i].name, Black); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
		SDL_Texture* Message = SDL_CreateTextureFromSurface(SDLRenderer, surfaceMessage); //now you can convert it into a texture
		SDL_Rect Message_rect; //create a rect
		Message_rect.x = worldClient->characterS[i].position.x + offset.x;  //controls the rect's x coordinate 
		Message_rect.y = worldClient->characterS[i].position.y - 50 + offset.y; // controls the rect's y coordinte
		Message_rect.w = squaresize; // controls the width of the rect
		Message_rect.h = 24; // controls the height of the rect

		//Mind you that (0,0) is on the top left of the window/screen, think a rect as the text's box, that way it would be very simple to understand

		//Now since it's a texture, you have to put RenderCopy in your game loop area, the area where the whole code executes

		SDL_RenderCopy(SDLRenderer, Message, NULL, &Message_rect); //you put the renderer's name first, the Message, the crop size(you can ignore this if you don't want to dabble with cropping), and the rect which is the size and coordinate of your texture

		//Don't forget to free your surface and texture
		SDL_FreeSurface(surfaceMessage);
		SDL_DestroyTexture(Message);*/
	}

	//render
	SDL_RenderPresent(SDLRenderer);
}

//ClientReceive gets updates from server
//worldCopy is not used after return
void ClientReceive(WorldClient* worldCopy){
	ClientDraw(worldCopy);
}

void ClientStart(void){
	//mutex init
	mutex = SDL_CreateMutex();
	if (!mutex){
		SDL_Log("ClientStart: mutex create: %s", SDL_GetError());
		exit(1);
	}

	//userClient create
	//not critical section
	userClient = UserClientNew();

	//userClient load
	strcpy(userClient->name, "asd"); //load abstraction
}

//ClientStop
void ClientStop(void){
	if(!SDL_RemoveTimer(tickId)){
		SDL_Log("ClientStop: SDL_RemoveTimer: %s", SDL_GetError());
		exit(1);
	}

	//wait timers to finish
	if (SDL_LockMutex(mutex) != 0){
		SDL_Log("ClientStop: SDL_LockMutex: %s", SDL_GetError());
		exit(1);
	}

	networkClientStop();

	//abilitySFree(userClient->ablityS);
	UserClientDelete(userClient);

	SDL_DestroyMutex(mutex);
}
