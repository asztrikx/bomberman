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
#include "config.h"
#include "network.h"

static SDL_mutex* mutex;
static UserClient* userClient;
static int tickId;

//Tick draws new frame
static Uint32 Tick(Uint32 interval, void *param){
	if (SDL_LockMutex(mutex) != 0){
		SDL_Log("ClientSend: SDL_LockMutex: %s", SDL_GetError());
		exit(1);
	}
	
	NetworkSendServer(userClient);

	if(SDL_UnlockMutex(mutex) < 0){
		SDL_Log("ClientSend: mutex unlock: %s", SDL_GetError());
		exit(1);
	}

	return interval;
}

//EventKey handles movement key events
static void EventKey(SDL_Event sdl_event){
	if (SDL_LockMutex(mutex) != 0){
		SDL_Log("EventKey: SDL_LockMutex: %s", SDL_GetError());
		exit(1);
	}

	bool activated = sdl_event.type == SDL_KEYDOWN;
	switch (sdl_event.key.keysym.sym){
		case SDLK_w:
			userClient->keyS[KeyUp] = activated;
			break;
		case SDLK_d:
			userClient->keyS[KeyRight] = activated;
			break;
		case SDLK_s:
			userClient->keyS[KeyDown] = activated;
			break;
		case SDLK_a:
			userClient->keyS[KeyLeft] = activated;
			break;
		case SDLK_SPACE:
			userClient->keyS[KeyBomb] = activated;
			break;
	}

	if(SDL_UnlockMutex(mutex) < 0){
		SDL_Log("EventKey: mutex unlock: %s", SDL_GetError());
		exit(1);
	}
}

//ClientConnect connects to a server
void ClientConnect(void){
	NetworkClientStart();

	NetworkConnectServer(userClient); //not critical section

	//server updater
	tickId = SDL_AddTimer(tickRate, Tick, NULL);
	if (tickId == 0){
		SDL_Log("SDL_AddTimer: %s", SDL_GetError());
		exit(1);
	}

	//key press
	SDL_Event sdl_event;
	while (SDL_WaitEvent(&sdl_event) && sdl_event.type != SDL_QUIT) {
		if(sdl_event.type == SDL_KEYDOWN || sdl_event.type == SDL_KEYUP){
			EventKey(sdl_event);
		}
	}
}

//DrawCharacterFind find CharacterYou
static Character* DrawCharacterFind(WorldClient* worldClient){
	//character find
	Character* characterMe = NULL;
	for(int i=0; i<worldClient->characterSLength; i++){
		if(worldClient->characterS[i].type == CharacterTypeYou){
			characterMe = &(worldClient->characterS[i]);
			break;
		}
	}

	return characterMe;
}

//DrawCharacter draws gameend screen
static void DrawGameend(WorldClient* worldClient){
	int r,g,b;
	switch (worldClient->gamestate){
		case GamestateDead:
			r = 255;
			g = 0;
			b = 0;
			break;
		case GamestateWon:
			r = 255;
			g = 255;
			b = 0;
			break;
		default:
			SDL_Log("DrawGameend: Unknown gamestate");
			exit(1);
			break;
	}

	if(SDL_SetRenderDrawColor(SDLRenderer, r, g, b, 255) < 0){
		SDL_Log("DrawGameend: SDL_SetRenderDrawColor: %s", SDL_GetError());
		exit(1);
	}
	if(SDL_RenderClear(SDLRenderer) < 0){
		SDL_Log("DrawGameend: SDL_RenderClear: %s", SDL_GetError());
		exit(1);
	}

	//render
	SDL_RenderPresent(SDLRenderer);
}

//DrawCharacter draws exit
static void DrawExit(WorldClient* worldClient, Position offset){
	if(worldClient->exit != NULL){
		Array* array = TextureSSObject[ObjectTypeExit];

		if(worldClient->exit->animation.state > array->length){
			SDL_Log("DrawExit: overindex");
			exit(1);
		}
		SDL_Texture* texture = array->data[worldClient->exit->animation.state];

		if(SDL_RenderCopy(SDLRenderer, texture, NULL, &(SDL_Rect){
			.y = worldClient->exit->position.y + offset.y,
			.x = worldClient->exit->position.x + offset.x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("DrawExit: SDL_RenderCopy: %s", SDL_GetError());
			exit(1);
		}
	}

}

//DrawCharacter draws objects
static void DrawObject(WorldClient* worldClient, Position offset){
	for(int i=0; i<worldClient->objectSLength; i++){
		Array* array = TextureSSObject[worldClient->objectS[i].type];

		if(worldClient->objectS[i].animation.state > array->length){
			SDL_Log("DrawObject: overindex");
			exit(1);
		}
		SDL_Texture* texture = array->data[worldClient->objectS[i].animation.state];

		if(SDL_RenderCopy(SDLRenderer, texture, NULL, &(SDL_Rect){
			.y = worldClient->objectS[i].position.y + offset.y,
			.x = worldClient->objectS[i].position.x + offset.x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("DrawObject: SDL_RenderCopy: %s", SDL_GetError());
			exit(1);
		}
	}
}

//DrawCharacter draws characters
static void DrawCharacter(WorldClient* worldClient, Position offset){
	for(int i=0; i<worldClient->characterSLength; i++){
		Array* array = TextureSSCharacter[worldClient->characterS[i].type];

		if(worldClient->characterS[i].animation.state > array->length){
			SDL_Log("DrawCharacter: overindex");
			exit(1);
		}
		SDL_Texture* texture = array->data[worldClient->characterS[i].animation.state];

		if(SDL_RenderCopy(SDLRenderer, texture, NULL, &(SDL_Rect){
			.y = worldClient->characterS[i].position.y + offset.y,
			.x = worldClient->characterS[i].position.x + offset.x,
			.w = squaresize,
			.h = squaresize,
		}) < 0){
			SDL_Log("DrawCharacter: SDL_RenderCopy: %s", SDL_GetError());
			exit(1);
		}
	}
}

//Draw draws to SDLRenderer
static void Draw(WorldClient* worldClient){
	//dead or won
	if(worldClient->gamestate != GamestateRunning){
		DrawGameend(worldClient);
		return;
	}

	Character* characterMe = DrawCharacterFind(worldClient);
	if(characterMe == NULL){
		SDL_Log("Draw: Did not receive character from server");
		exit(1);
	}

	//offset
	Position offset = (Position){
		.y = -characterMe->position.y + windowHeight / 2,
		.x = -characterMe->position.x + windowWidth / 2,
	};

	//clear & background
	if(SDL_SetRenderDrawColor(SDLRenderer, 0, 255, 0, 255) < 0){
		SDL_Log("Draw: SDL_SetRenderDrawColor: %s", SDL_GetError());
		exit(1);
	}
	if(SDL_RenderClear(SDLRenderer) < 0){
		SDL_Log("Draw: SDL_RenderClear: %s", SDL_GetError());
		exit(1);
	}

	DrawExit(worldClient, offset);

	DrawObject(worldClient, offset);

	DrawCharacter(worldClient, offset);

	SDL_RenderPresent(SDLRenderer);
}

//ClientReceive gets updates from server
//worldCopy is not used after return
void ClientReceive(WorldClient* worldCopy){
	Draw(worldCopy);
}

//ClientStop loads client module
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

//ClientStop unloads client module
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

	NetworkClientStop();

	//abilitySFree(userClient->ablityS);
	UserClientDelete(userClient);

	SDL_DestroyMutex(mutex);
}
