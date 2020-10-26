#include "debugmalloc.h"
#include "server.h"
#include <stdbool.h>
#include "state.h"
#include "geometry.h"
#include "network.h"

//mutex handles all global variable which is modified in critical sections
static SDL_mutex* mutex;
static UserServerItem* userServerItemS;
static WorldServer* worldServer;
static long long tickCount = 0;
static unsigned int tickRate = 60u;
static int tickId;

UserServer* ServerAuthCheck(char* auth){
	UserServerItem* userServerItemCurrent = userServerItemS;
	while(userServerItemCurrent != NULL){
		//connecting user
		if(userServerItemCurrent->userServer.auth == NULL){
			userServerItemCurrent = userServerItemCurrent->next;
			continue;
		}

		//timing attack safe compare
		bool diff = false;
		for(int i=0; i<26; i++){
			if(auth[i] != userServerItemCurrent->userServer.auth[i]){
				diff = true;
			}
		}
		if(!diff){
			return &(userServerItemCurrent->userServer);
		}

		userServerItemCurrent = userServerItemCurrent->next;
	}

	return NULL;
}

//ServerAuthCreate creates a 26 character long auth key
//return should be free'd by caller
char* ServerAuthCreate(){
	char* auth = (char*) malloc((26 + 1) * sizeof(char));
	for(int i=0; i<26; i++){
		auth[i] = rand() % ('Z' - 'A' + 1) + 'A';
	}
	auth[26] = '\0';

	return auth;
}

//ServerTick calculates new frame, notifies users
Uint32 ServerTick(Uint32 interval, void *param){
	if(SDL_LockMutex(mutex) != 0){
		SDL_Log("ServerTick: mutex lock: %s", SDL_GetError());
		exit(1);
	}

	//[R]bomb explosion
	//[R]player death
	//[R]crate delete

	//user notify
	UserServerItem* userServerItemCurrent = userServerItemS;
	while(userServerItemCurrent != NULL){
		//alter user character to be identifiable
		userServerItemCurrent->userServer.character->type = CharacterTypeYou;		
		networkSendClient(worldServer);
		userServerItemCurrent->userServer.character->type = CharacterTypeUser;

		userServerItemCurrent = userServerItemCurrent->next;
	}

	if(SDL_UnlockMutex(mutex) < 0){
		SDL_Log("ServerTick: mutex unlock: %s", SDL_GetError());
		exit(1);
	}

	return interval;
}

//ServerStart generates world, start accepting connections, starts ticking
void ServerStart(void){
	//world generate
	worldServer = worldGenerate(11, 11); //not critical section

	//mutex init
	mutex = SDL_CreateMutex();
	if (!mutex){
		SDL_Log("mutex create: %s", SDL_GetError());
		exit(1);
	}

	//network start
	networkServerStart();

	//tick start: world calc, connected user update
	tickId = SDL_AddTimer(1000u/tickRate, ServerTick, NULL);
	if (tickId == 0){
		SDL_Log("SDL_AddTimer: %s", SDL_GetError());
		exit(1);
	}
}

//keyMovement calculates user position based on keyItem.key
void keyMovement(SDL_Keycode key, UserServer* userServer){
	if(
		key != SDLK_w &&
		key != SDLK_a &&
		key != SDLK_s &&
		key != SDLK_d
	){
		return;
	}

	Position positionNew = userServer->character->position;
	if(key == SDLK_w){
		positionNew.y -= userServer->character->velocity.y;
	} else if(key == SDLK_a){
		positionNew.x -= userServer->character->velocity.x;
	} else if(key == SDLK_s){
		positionNew.y += userServer->character->velocity.y;
	} else if(key == SDLK_d){
		positionNew.x += userServer->character->velocity.x;
	}

	if(
		collisionObjectS(worldServer->objectItemS, positionNew) ||
		collisionCharacterS(worldServer->characterItemS, positionNew, userServer->character)
	){
		return;
	}
	
	userServer->character->position = positionNew;
}

//keyBomb calculates bomb position based on keyItem.key
void keyBomb(SDL_Keycode key, UserServer* userServer){
	if(key != SDLK_SPACE){
		return;
	}

	Position positionNew = userServer->character->position;

	//position
	if (positionNew.y % squaresize > squaresize / 2){
		positionNew.y += squaresize;
	}
	if (positionNew.x % squaresize > squaresize / 2){
		positionNew.x += squaresize;
	}
	positionNew.y -= positionNew.y % squaresize;
	positionNew.x -= positionNew.x % squaresize;

	//collision
	if (
		collisionObjectS(worldServer->objectItemS, positionNew) ||
		collisionCharacterS(worldServer->characterItemS, positionNew, userServer->character)
	){
		return;
	}

	//bomb create
	Object object = (Object) {
		.created = tickCount,
		.position = positionNew,
		.type = ObjectTypeBomb,
		.velocity = (Position){0, 0},
	};

	//bomb insert
	ObjectItem* objectItem = (ObjectItem*) malloc(sizeof(Object));
	objectItem->object = object;

	if(worldServer->objectItemS == NULL){ //first in list
		objectItem->prev = NULL;
		objectItem->next = NULL;
		
		worldServer->objectItemS = objectItem;
	} else {
		objectItem->prev = NULL;
		objectItem->next = worldServer->objectItemS;
		
		worldServer->objectItemS->prev = objectItem;

		worldServer->objectItemS = objectItem;
	}
}

//ServerReceive gets updates from users
void ServerReceive(UserServer* userServerUnsafe){
	if (SDL_LockMutex(mutex) != 0){
		puts("ServerReceive: mutex lock");
		exit(1);
	}

	//auth validate
	UserServer* userServer = ServerAuthCheck(userServerUnsafe->auth); //[R] auth may be shorther than 26
	if(userServer == NULL){
		return;
	}

	//name change
	if(
		userServerUnsafe != NULL &&
		strncmp(userServer->name, userServerUnsafe->name, 15) != 0
	){
		strncpy(userServer->name, userServerUnsafe->name, 15);
	}

	//keyS apply
	for(int i=0; i<userServerUnsafe->keySLength; i++){
		keyBomb(userServerUnsafe->keyS[i], userServer);
		keyMovement(userServerUnsafe->keyS[i], userServer);
	}

	SDL_UnlockMutex(mutex);
}

//ServerStop clears server module
void ServerStop(void){
	if (SDL_LockMutex(mutex) != 0){
		puts("ServerStop: mutex lock");
		exit(1);
	}

	networkServerStop();
	SDL_RemoveTimer(tickId);

	//exit
	free(worldServer->exit);
	
	//characterItems
	CharacterItem* characterItemCurrent = worldServer->characterItemS;
	CharacterItem* characterItemPrev;
	while(characterItemCurrent != NULL){
		characterItemPrev = characterItemCurrent;
		characterItemCurrent = characterItemCurrent->next;

		//free(characterItemPrev->character.name); //it a pointer to userServer
		free(characterItemPrev);
	}

	//objectItems
	ObjectItem* objectItemCurrent = worldServer->objectItemS;
	ObjectItem* objectItemPrev;
	while(objectItemCurrent != NULL){
		objectItemPrev = objectItemCurrent;
		objectItemCurrent = objectItemCurrent->next;

		free(objectItemPrev);
	}

	//worldServer
	free(worldServer);

	//userItemS
	UserServerItem* userServerItemCurrent = userServerItemS;
	UserServerItem* userServerItemPrev;
	while(userServerItemCurrent != NULL){
		userServerItemPrev = userServerItemCurrent;
		userServerItemCurrent = userServerItemCurrent->next;

		free(userServerItemPrev->userServer.auth);
		free(userServerItemPrev->userServer.name);
		free(userServerItemPrev->userServer.keyS); //NULL
		free(userServerItemPrev);
	}

	SDL_DestroyMutex(mutex);
}

//ServerConnect register new connection user, returns it with auth
void ServerConnect(UserServer* userServerUnsafe){
	//[R]check if game is running

	if (SDL_LockMutex(mutex) != 0){
		SDL_Log("ServerConnect: mutex lock: %s", SDL_GetError());
		exit(1);
	}

	//userServer insert
	UserServerItem* userServerItem = userServerItemSInsert(&userServerItemS, &(UserServer){
		.auth = NULL,
		.character = NULL,
		.keyS = NULL,
		.keySLength = 0,
		.name = (char*) malloc((15 + 1) * sizeof(char)),
	});
	UserServer* userServer = &(userServerItem->userServer);
	strncpy(userServer->name, userServerUnsafe->name, 15);
	userServer->name[16] = '\0';

	//id generate
	while (true){
		char* auth = ServerAuthCreate();
		
		//id exists
		if(ServerAuthCheck(auth) == NULL){
			userServer->auth = auth;
			break;
		}

		free(auth);
	}

	//character insert
	CharacterItem* characterItem = characterItemSInsert(&(worldServer->characterItemS), &(Character){
		.bomb = 1,
		.name = userServer->name,
		.position = (Position) {1 * squaresize, 1 * squaresize},
		.type = CharacterTypeUser,
		.velocity = velocity,
	});

	//character associate
	userServer->character = &(characterItem->character);

	//reply
	free(userServerUnsafe->auth); //in best case it's free(NULL)
	userServerUnsafe->auth = (char*) malloc((26 + 1) * sizeof(char));
	strcpy(userServerUnsafe->auth, userServer->auth);

	if(SDL_UnlockMutex(mutex) < 0){
		SDL_Log("ServerConnect: mutex unlock: %s", SDL_GetError());
		exit(1);
	}
}
