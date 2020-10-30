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
		SDL_Log("ServerTick: SDL_LockMutex: %s", SDL_GetError());
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

	ObjectItem* objectItemCollisionS = collisionObjectS(worldServer->objectItemS, userServer->character->position, positionNew);
	CharacterItem* characterItemCollisionS = collisionCharacterS(worldServer->characterItemS, userServer->character->position, positionNew);

	if (
		characterItemCollisionS != NULL &&
		(
			characterItemCollisionS->next != NULL ||
			&(characterItemCollisionS->character) != userServer->character
		)
	){
		objectItemSFree(objectItemCollisionS);
		characterItemSFree(characterItemCollisionS);
		return;
	}
	if(objectItemCollisionS != NULL){
		//player can be inside bomb
		//it can be inside two bomb
		//eg: inside firstly placed one and just placed one into neighbourgh position
		ObjectItem* objectItemCurrent = objectItemCollisionS;
		while(objectItemCurrent != NULL){
			if(
				objectItemCurrent->object.type != ObjectTypeBomb ||
				objectItemCurrent->object.owner != userServer->character ||
				objectItemCurrent->object.bombOut
			){
				objectItemSFree(objectItemCollisionS);
				characterItemSFree(characterItemCollisionS);
				return;
			}

			objectItemCurrent = objectItemCurrent->next;
		}
	}

	//moved from an area with its own bombs
	if(objectItemCollisionS != NULL){
		ObjectItem* objectItemCurrent = objectItemCollisionS;
		while(objectItemCurrent != NULL){
			//bombs which to player can not move back
			//(it can be that it moved out from it in the past)
			if(collisionPoint(positionNew, objectItemCurrent->object.position)){
				objectItemCurrent->object.bombOut = true;
			}

			objectItemCurrent = objectItemCurrent->next;
		}
	}
	
	userServer->character->position = positionNew;

	objectItemSFree(objectItemCollisionS);
	characterItemSFree(characterItemCollisionS);
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


	ObjectItem* objectItemCollisionS = collisionObjectS(worldServer->objectItemS, userServer->character->position, positionNew);
	CharacterItem* characterItemCollisionS = collisionCharacterS(worldServer->characterItemS, userServer->character->position, positionNew);

	if (characterItemCollisionS != NULL){
		if(
			characterItemCollisionS->next != NULL ||
			&(characterItemCollisionS->character) != userServer->character
		){
			objectItemSFree(objectItemCollisionS);
			characterItemSFree(characterItemCollisionS);
			return;
		}
	}
	if (objectItemCollisionS != NULL){
		objectItemSFree(objectItemCollisionS);
		characterItemSFree(characterItemCollisionS);
		return;
	}

	//bomb insert
	Object object = (Object){
		.created = tickCount,
		.position = positionNew,
		.type = ObjectTypeBomb,
		.velocity = (Position){0, 0},
		.bombOut = false,
		.owner = userServer->character,
	};
	objectItemSInsert(&(worldServer->objectItemS), &object);

	objectItemSFree(objectItemCollisionS);
	characterItemSFree(characterItemCollisionS);
}

//ServerReceive gets updates from users
//userServerUnsafe is not used after return
void ServerReceive(UserServer* userServerUnsafe){
	if (SDL_LockMutex(mutex) != 0){
		SDL_Log("ServerReceive: SDL_LockMutex: %s", SDL_GetError());
		exit(1);
	}

	//auth validate
	UserServer* userServer = ServerAuthCheck(userServerUnsafe->auth); //[R] auth may be shorther than 26
	if(userServer == NULL){
		return;
	}

	//name change
	if(strncmp(userServer->name, userServerUnsafe->name, 15) != 0){
		strncpy(userServer->name, userServerUnsafe->name, 15);
		userServer->name[15] = '\0'; //in best case it's already padded
	}

	//keyS apply
	for(int i=0; i<userServerUnsafe->keySLength; i++){ //[R] keySLength may be falsified
		keyBomb(userServerUnsafe->keyS[i], userServer);
		keyMovement(userServerUnsafe->keyS[i], userServer);
	}

	if(SDL_UnlockMutex(mutex) < 0){
		SDL_Log("ServerReceive: mutex unlock: %s", SDL_GetError());
		exit(1);
	}
}

//ServerStop clears server module
void ServerStop(void){
	if(!SDL_RemoveTimer(tickId)){
		SDL_Log("ServerStop: SDL_RemoveTimer: %s", SDL_GetError());
		exit(1);
	}

	//wait timers to finish
	if (SDL_LockMutex(mutex) != 0){
		SDL_Log("ServerStop: SDL_LockMutex: %s", SDL_GetError());
		exit(1);
	}

	networkServerStop();

	//free worldServer
	free(worldServer->exit);
	characterItemSFree(worldServer->characterItemS);
	objectItemSFree(worldServer->objectItemS);
	free(worldServer);

	//free userItemS
	userServerItemSFree(userServerItemS);

	SDL_DestroyMutex(mutex);
}

//ServerConnect register new connection user, returns it with auth
//userServerUnsafe is not used after return
void ServerConnect(UserServer* userServerUnsafe){
	//[R]check if game is running

	if (SDL_LockMutex(mutex) != 0){
		SDL_Log("ServerConnect: SDL_LockMutex: %s", SDL_GetError());
		exit(1);
	}

	//userServer insert
	UserServer userServerCopy = (UserServer){
		.auth = NULL,
		.character = NULL,
		.keyS = NULL,
		.keySLength = 0,
		.name = (char*) malloc((15 + 1) * sizeof(char)),
	};
	UserServerItem* userServerItem = userServerItemSInsert(&userServerItemS, &userServerCopy);
	UserServer* userServer = &(userServerItem->userServer);
	strncpy(userServer->name, userServerUnsafe->name, 15);
	userServer->name[15] = '\0';

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
	Character character = (Character){
		.bomb = 1,
		.name = userServer->name,
		.position = (Position) {1 * squaresize, 1 * squaresize},
		.type = CharacterTypeUser,
		.velocity = velocity,
	};
	CharacterItem* characterItem = characterItemSInsert(&(worldServer->characterItemS), &character);

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
