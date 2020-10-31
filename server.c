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
static const unsigned int tickRate = 60u;
static const long long tickSecond = tickRate; //tick count in one second
static int tickId;

void keyMovement(SDL_Keycode key, UserServer* userServer);
void keyBomb(SDL_Keycode key, UserServer* userServer);

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

void bombExplode(Object* object){
	if(object->type != ObjectTypeBomb){
		return;
	}

	//fire add
	int radius = 1;

	//fire insert
	//[R] collision with walls
	int directionX[] = {1, -1, 0, 0};
	int directionY[] = {0, 0, 1, -1};
	for(int i=0; i<=radius; i++){
		for(int j=0; j<4; j++){
			Object objectFire = (Object){
				.bombOut = true,
				.created = tickCount,
				.destroy = tickCount + 0.25 * tickSecond,
				.owner = object->owner,
				.position = (Position){
					.y = object->position.y + i * directionY[j] * squaresize,
					.x = object->position.x + i * directionX[j] * squaresize,
				},
				.type = ObjectTypeBombFire,
				.velocity = (Position) {
					.y = 0,
					.x = 0,
				},
			};
			objectItemSInsert(&(worldServer->objectItemS), &objectFire);

			//otherwise there would be 4 fire in the same spot
			if(i == 0){
				break;
			}
		}
	}

	//bomb remove
	if(object->owner != NULL){
		object->owner->bombCount--;
	}
}

void fireDestroy(Object* object){
	if(object->type != ObjectTypeBombFire){
		return;
	}

	//object collision
	ObjectItem* objectItemCollisionS = collisionObjectS(worldServer->objectItemS, object->position, object->position);

	ObjectItem* objectItemCollisionCurrent = objectItemCollisionS;
	while(objectItemCollisionCurrent != NULL){
		if(objectItemCollisionCurrent->object->type == ObjectTypeBox){
			ObjectItem* objectItem = objectItemSFind(worldServer->objectItemS, objectItemCollisionCurrent->object);
			objectItemSRemove(&(worldServer->objectItemS), objectItem, true);
		} else if(objectItemCollisionCurrent->object->type == ObjectTypeBomb){
			//bombExplode(objectItemCurrent->object);
		}

		objectItemCollisionCurrent = objectItemCollisionCurrent->next;
	}
	objectItemSFree(objectItemCollisionS, false);

	//character collision
	CharacterItem* characterItemCollisionS = collisionCharacterS(worldServer->characterItemS, object->position, object->position);

	CharacterItem* characterItemCollisionCurrent = characterItemCollisionS;
	while(characterItemCollisionCurrent != NULL){
		CharacterItem* characterItem = characterItemSFind(worldServer->characterItemS, characterItemCollisionCurrent->character);
		characterItemSRemove(&(worldServer->characterItemS), characterItem, true);

		characterItemCollisionCurrent = characterItemCollisionCurrent->next;
	}
	characterItemSFree(characterItemCollisionS, false);
}

//ServerTick calculates new frame, notifies users
Uint32 ServerTick(Uint32 interval, void *param){
	if(SDL_LockMutex(mutex) != 0){
		SDL_Log("ServerTick: SDL_LockMutex: %s", SDL_GetError());
		exit(1);
	}

	//destroy by server
	//this should be calculated first as these objects should not exists in this tick
	ObjectItem* objectItemCurrent = worldServer->objectItemS;
	while(objectItemCurrent != NULL){
		if(tickCount != objectItemCurrent->object->destroy){
			objectItemCurrent = objectItemCurrent->next;
			continue;
		}

		bombExplode(objectItemCurrent->object);

		objectItemCurrent = objectItemCurrent->next;

		objectItemSRemove(&(worldServer->objectItemS), objectItemCurrent->prev, true);
	}

	//player movement
	//this should be calculated before fireDestroy() otherwise player would be in fire for 1 tick
	UserServerItem* userServerItemCurrent = userServerItemS;
	while(userServerItemCurrent != NULL){
		for (int i=0; i<userServerItemCurrent->userServer.keySLength; i++){
			keyBomb(userServerItemCurrent->userServer.keyS[i], &(userServerItemCurrent->userServer));
			keyMovement(userServerItemCurrent->userServer.keyS[i], &(userServerItemCurrent->userServer));
		}
		
		userServerItemCurrent = userServerItemCurrent->next;
	}

	//destroy by user
	objectItemCurrent = worldServer->objectItemS;
	while(objectItemCurrent != NULL){
		fireDestroy(objectItemCurrent->object);

		objectItemCurrent = objectItemCurrent->next;
	}

	//[R]state change

	//user notify
	userServerItemCurrent = userServerItemS;
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

	tickCount++;
	return interval;
}

//ServerStart generates world, start accepting connections, starts ticking
void ServerStart(void){
	//world generate
	worldServer = worldGenerate(17, 57); //not critical section

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

	//[R] collision should not drop position new, just cut it &positionNew to functions

	if (
		characterItemCollisionS != NULL &&
		(
			characterItemCollisionS->next != NULL ||
			characterItemCollisionS->character != userServer->character
		)
	){
		objectItemSFree(objectItemCollisionS, false);
		characterItemSFree(characterItemCollisionS, false);
		return;
	}
	if(objectItemCollisionS != NULL){
		ObjectItem* objectItemCurrent = objectItemCollisionS;
		while(objectItemCurrent != NULL){
			//player can be inside fire (it will die in this exact tick)
			if(objectItemCurrent->object->type == ObjectTypeBombFire){
				objectItemCurrent = objectItemCurrent->next;
				continue;
			}

			//player can be inside bomb
			//it can be inside two bomb
			//eg: inside firstly placed one and just placed one into neighbourgh position
			if(
				objectItemCurrent->object->type == ObjectTypeBomb &&
				objectItemCurrent->object->owner == userServer->character &&
				!objectItemCurrent->object->bombOut
			){
				objectItemCurrent = objectItemCurrent->next;
				continue;
			}

			objectItemSFree(objectItemCollisionS, false);
			characterItemSFree(characterItemCollisionS, false);
			return;
		}
	}

	//moved from an area with its own bombs
	if(objectItemCollisionS != NULL){
		ObjectItem* objectItemCurrent = objectItemCollisionS;
		while(objectItemCurrent != NULL){
			//bombs which to player can not move back
			//(it can be that it moved out from it in the past)
			if(!collisionPoint(positionNew, objectItemCurrent->object->position)){
				objectItemCurrent->object->bombOut = true;
			}

			objectItemCurrent = objectItemCurrent->next;
		}
	}
	
	userServer->character->position = positionNew;

	objectItemSFree(objectItemCollisionS, false);
	characterItemSFree(characterItemCollisionS, false);
}

//keyBomb calculates bomb position based on keyItem.key
void keyBomb(SDL_Keycode key, UserServer* userServer){
	if(key != SDLK_SPACE){
		return;
	}

	//[R]bomb available

	Position positionNew = userServer->character->position;

	//position
	positionNew.y -= positionNew.y % squaresize;
	positionNew.x -= positionNew.x % squaresize;
	if (userServer->character->position.y % squaresize > squaresize / 2){
		positionNew.y += squaresize;
	}
	if (userServer->character->position.x % squaresize > squaresize / 2){
		positionNew.x += squaresize;
	}

	ObjectItem* objectItemCollisionS = collisionObjectS(worldServer->objectItemS, positionNew, positionNew);
	CharacterItem* characterItemCollisionS = collisionCharacterS(worldServer->characterItemS, positionNew, positionNew);

	if (characterItemCollisionS != NULL){
		if(
			characterItemCollisionS->next != NULL ||
			characterItemCollisionS->character != userServer->character
		){
			objectItemSFree(objectItemCollisionS, false);
			characterItemSFree(characterItemCollisionS, false);
			return;
		}
	}
	if (objectItemCollisionS != NULL){
		objectItemSFree(objectItemCollisionS, false);
		characterItemSFree(characterItemCollisionS, false);
		return;
	}

	//bomb insert
	Object object = (Object){
		.created = tickCount,
		.destroy = tickCount + 2 * tickSecond,
		.position = positionNew,
		.type = ObjectTypeBomb,
		.velocity = (Position){0, 0},
		.bombOut = false,
		.owner = userServer->character,
	};
	objectItemSInsert(&(worldServer->objectItemS), &object);

	objectItemSFree(objectItemCollisionS, false);
	characterItemSFree(characterItemCollisionS, false);
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

	//keyS copy
	//[R] keySLength may be falsified
	//[R] make values unique
	free(userServer->keyS);
	userServer->keySLength = userServerUnsafe->keySLength;
	userServer->keyS = (SDL_Keycode*) malloc(userServerUnsafe->keySLength * sizeof(SDL_Keycode));
	for(int i=0; i<userServerUnsafe->keySLength; i++){
		userServer->keyS[i] = userServerUnsafe->keyS[i];
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
	characterItemSFree(worldServer->characterItemS, true);
	objectItemSFree(worldServer->objectItemS, true);
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

	//position random
	Position positionCompressed = (Position){
		.y = 0,
		.x = 0,
	};
	Position position;
	while(
		positionCompressed.y == 0 ||
		positionCompressed.x == 0 ||
		positionCompressed.y == worldServer->height - 1 ||
		positionCompressed.x == worldServer->width - 1 ||
		(
			positionCompressed.y % 2 == 0 &&
			positionCompressed.x % 2 == 0
		) ||
		collisionObjectS(worldServer->objectItemS, position, position) != NULL ||
		collisionCharacterS(worldServer->characterItemS, position, position) != NULL
	){
		positionCompressed.y = rand() % worldServer->height;
		positionCompressed.x = rand() % worldServer->width;

		position.y = positionCompressed.y * squaresize;
		position.x = positionCompressed.x * squaresize;
	}

	//[R]clear area

	//character insert
	Character character = (Character){
		.bombCount = 1,
		.name = userServer->name,
		.position = (Position) {
			position.y,
			position.x
		},
		.type = CharacterTypeUser,
		.velocity = velocity,
	};
	CharacterItem* characterItem = characterItemSInsert(&(worldServer->characterItemS), &character);

	//character associate
	userServer->character = characterItem->character;

	//reply
	free(userServerUnsafe->auth); //in best case it's free(NULL)
	userServerUnsafe->auth = (char*) malloc((26 + 1) * sizeof(char));
	strcpy(userServerUnsafe->auth, userServer->auth);

	if(SDL_UnlockMutex(mutex) < 0){
		SDL_Log("ServerConnect: mutex unlock: %s", SDL_GetError());
		exit(1);
	}
}
