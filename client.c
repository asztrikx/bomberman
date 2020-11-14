#include "debugmalloc.h"
#include "client.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <stdbool.h>
#include "SDL.h"
#include "type/user/client.h"
#include "type/world/client.h"
#include "type/geometry.h"
#include "type/array.h"
#include "type/animation.h"
#include "type/int.h"
#include "config.h"
#include "network.h"

static SDL_mutex* mutex;
static UserClient* userClient;
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
			ListRemoveItem(&(userClient->keyList), item, IntDelete);
			break;
		}
	}

	//key not in list, add
	if(sdl_event.type == SDL_KEYDOWN && !in){
		int* key = IntNew();
		*key = sdl_event.key.keysym.sym;
		ListInsert(&(userClient->keyList), key);
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
	tickId = SDL_AddTimer(tickRate, ClientSend, NULL);
	if (tickId == 0){
		SDL_Log("SDL_AddTimer: %s", SDL_GetError());
		exit(1);
	}
}

Character* clientDrawCharacterFind(WorldClient* worldClient){
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
	}

	return characterMe;
}

void clientDrawExit(WorldClient* worldClient, Position offset){
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

}

void clientDrawObject(WorldClient* worldClient, Position offset){
	for(int i=0; i<worldClient->objectSLength; i++){
		Array* array = TextureSSObject[worldClient->objectS[i].type];

		if(worldClient->objectS[i].animation.state > array->length){
			SDL_Log("clientDrawObject: overindex");
			exit(1);
		}
		SDL_Texture* texture = array->data[worldClient->objectS[i].animation.state];

		if(SDL_RenderCopy(SDLRenderer, texture, NULL, &(SDL_Rect){
			.y = worldClient->objectS[i].position.y + offset.y,
			.x = worldClient->objectS[i].position.x + offset.x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("clientDrawObject: SDL_RenderCopy: %s", SDL_GetError());
			exit(1);
		}
	}
}

void clientDrawCharacter(WorldClient* worldClient, Position offset){
	for(int i=0; i<worldClient->characterSLength; i++){
		Array* array = TextureSSCharacter[worldClient->characterS[i].type];

		if(worldClient->characterS[i].animation.state > array->length){
			SDL_Log("clientDrawCharacter: overindex");
			exit(1);
		}
		SDL_Texture* texture = array->data[worldClient->characterS[i].animation.state];

		if(SDL_RenderCopy(SDLRenderer, texture, NULL, &(SDL_Rect){
			.y = worldClient->characterS[i].position.y + offset.y,
			.x = worldClient->characterS[i].position.x + offset.x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("clientDrawObject: SDL_RenderCopy: %s", SDL_GetError());
			exit(1);
		}
	}
}

void clientDrawCharacterName(WorldClient* worldClient, Position offset){
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
}

void ClientDraw(WorldClient* worldClient){
	//dead
	Character* characterMe = clientDrawCharacterFind(worldClient);
	if(characterMe == NULL){
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
	clientDrawExit(worldClient, offset);

	//object
	clientDrawObject(worldClient, offset);

	//character
	clientDrawCharacter(worldClient, offset);

	//character name
	clientDrawCharacterName(worldClient, offset);

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
