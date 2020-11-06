#include "debugmalloc.h"
#include "server.h"
#include <stdbool.h>
#include "state.h"
#include "geometry.h"
#include "network.h"

//mutex handles all global variable which is modified in critical sections
static SDL_mutex* mutex;
static List* userServerList;
static WorldServer* worldServer;
static long long tickCount = 0;
static const unsigned int tickRate = 60u;
static const long long tickSecond = tickRate; //tick count in one second
static int tickId;

//keyMovement calculates user position based on keyItem.key
//mutex must be locked
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

	List* listCollisionObject = collisionObjectS(worldServer->objectList, userServer->character->position, positionNew);
	List* listCollisionCharacter = collisionCharacterS(worldServer->characterList, userServer->character->position, positionNew);

	//[R] collision should not drop position new, just cut it &positionNew to functions

	if (
		listCollisionCharacter->length != 1 ||
		listCollisionCharacter->head->data != userServer->character
	){
		ListDelete(listCollisionObject, false);
		ListDelete(listCollisionCharacter, false);
		return;
	}
	if(listCollisionObject->length != 0){
		ListItem* listItemCurrent = listCollisionObject->head;
		while(listItemCurrent != NULL){
			//player can be inside fire (it will die in this exact tick)
			if(((Object*)listItemCurrent->data)->type == ObjectTypeBombFire){
				listItemCurrent = listItemCurrent->next;
				continue;
			}

			//player can be inside bomb
			//it can be inside two bomb
			//eg: inside firstly placed one and just placed one into neighbourgh position
			if(
				((Object*)listItemCurrent->data)->type == ObjectTypeBomb &&
				((Object*)listItemCurrent->data)->owner == userServer->character &&
				!((Object*)listItemCurrent->data)->bombOut
			){
				listItemCurrent = listItemCurrent->next;
				continue;
			}

			ListDelete(listCollisionObject, false);
			ListDelete(listCollisionCharacter, false);
			return;
		}
	}

	//moved from an area with its own bombs
	if(listCollisionObject->length != 0){
		ListItem* listItemCurrent = listCollisionObject->head;
		while(listItemCurrent != NULL){
			//bombs which to player can not move back
			//(it can be that it moved out from it in the past)
			if(!collisionPoint(positionNew, ((Object*)listItemCurrent->data)->position)){
				((Object*)listItemCurrent->data)->bombOut = true;
			}

			listItemCurrent = listItemCurrent->next;
		}
	}
	
	userServer->character->position = positionNew;

	ListDelete(listCollisionObject, false);
	ListDelete(listCollisionCharacter, false);
}

//keyBomb calculates bomb position based on keyItem.key
//mutex must be locked
void keyBomb(SDL_Keycode key, UserServer* userServer){
	if(key != SDLK_SPACE){
		return;
	}

	//bomb available
	if(userServer->character->bombCount == 0){
		return;
	}

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

	List* listCollisionObject = collisionObjectS(worldServer->objectList, positionNew, positionNew);
	List* listCollisionCharacter  = collisionCharacterS(worldServer->characterList, positionNew, positionNew);

	if (listCollisionCharacter->head != NULL){
		if(
			listCollisionCharacter->head->next != NULL ||
			listCollisionCharacter->head->data != userServer->character
		){
			ListDelete(listCollisionObject, false);
			ListDelete(listCollisionCharacter, false);
			return;
		}
	}
	if (listCollisionObject->head != NULL){
		ListDelete(listCollisionObject, false);
		ListDelete(listCollisionCharacter, false);
		return;
	}

	//bomb insert
	ListInsert(&(worldServer->objectList), Copy(&(Object){
		.created = tickCount,
		.destroy = tickCount + 2 * tickSecond,
		.position = positionNew,
		.type = ObjectTypeBomb,
		.velocity = (Position){0, 0},
		.bombOut = false,
		.owner = userServer->character,
	}, sizeof(Object)));

	//free
	ListDelete(listCollisionObject, false);
	ListDelete(listCollisionCharacter, false);

	//bomb decrease
	userServer->character->bombCount--;
}

UserServer* ServerAuthCheck(char* auth){
	ListItem* listItemCurrent = userServerList->head;
	while(listItemCurrent != NULL){
		//connecting user
		if(((UserServer*)listItemCurrent->data)->auth == NULL){
			listItemCurrent = listItemCurrent->next;
			continue;
		}

		//timing attack safe compare
		bool diff = false;
		for(int i=0; i<26; i++){
			if(auth[i] != ((UserServer*)listItemCurrent->data)->auth[i]){
				diff = true;
			}
		}
		if(!diff){
			return (UserServer*)(listItemCurrent->data);
		}

		listItemCurrent = listItemCurrent->next;
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
			ListInsert(&(worldServer->objectList), Copy(&(Object){
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
			}, sizeof(Object)));

			//otherwise there would be 4 fire in the same spot
			if(i == 0){
				break;
			}
		}
	}

	//bomb remove
	if(object->owner != NULL){
		object->owner->bombCount++;
	}
}

void fireDestroy(Object* object){
	if(object->type != ObjectTypeBombFire){
		return;
	}

	//object collision
	List* listCollisionObject = collisionObjectS(worldServer->objectList, object->position, object->position);

	ListItem* listItemCurrent = listCollisionObject->head;
	while(listItemCurrent != NULL){
		if(((Object*)listItemCurrent->data)->type == ObjectTypeBox){
			ListItem* listItem = ListFindItem(worldServer->objectList, listItemCurrent->data);
			ListRemoveItem(&(worldServer->objectList), listItem, ObjectDelete);
		} else if(((Object*)listItemCurrent->data)->type == ObjectTypeBomb){
			//bombExplode(objectItemCurrent->object); [R]
		}

		listItemCurrent = listItemCurrent->next;
	}
	ListDelete(listCollisionObject, false);

	//character collision
	List* listCollisionCharacter = collisionObjectS(worldServer->characterList, object->position, object->position);

	listItemCurrent = listCollisionCharacter->head;
	while(listItemCurrent != NULL){
		ListItem* listItem = ListFindItem(worldServer->characterList, listItemCurrent->data);
		ListRemoveItem(&(worldServer->characterList), listItem, CharacterDelete);

		listItemCurrent = listItemCurrent->next;
	}
	ListDelete(listCollisionCharacter, false);
}

//ServerTick calculates new frame, notifies users
Uint32 ServerTick(Uint32 interval, void *param){
	if(SDL_LockMutex(mutex) != 0){
		SDL_Log("ServerTick: SDL_LockMutex: %s", SDL_GetError());
		exit(1);
	}

	//destroy by server
	//this should be calculated first as these objects should not exists in this tick
	ListItem* listItemCurrent = worldServer->objectList->head;
	while(listItemCurrent != NULL){
		if(tickCount != ((Object*)listItemCurrent->data)->destroy){
			listItemCurrent = listItemCurrent->next;
			continue;
		}

		bombExplode(listItemCurrent->data);

		listItemCurrent = listItemCurrent->next;

		ListRemoveItem(&(worldServer->objectList), listItemCurrent->prev, ObjectDelete);
	}

	//player movement
	//this should be calculated before fireDestroy() otherwise player would be in fire for 1 tick
	listItemCurrent = userServerList->head;
	while(listItemCurrent != NULL){
		for (int i=0; i<((UserServer*)listItemCurrent->data)->keySLength; i++){
			//if(userServerItemCurrent->userServer != NULL){ [R]
				//should be before keyMovement as user wants to place bomb on the position currently seeable
				keyBomb(((UserServer*)listItemCurrent->data)->keyS[i], listItemCurrent->data);
				keyMovement(((UserServer*)listItemCurrent->data)->keyS[i], listItemCurrent->data);
			//}
		}
		
		listItemCurrent = listItemCurrent->next;
	}

	//destroy by user
	listItemCurrent = worldServer->objectList->head;
	while(listItemCurrent != NULL){
		fireDestroy(listItemCurrent->data);

		listItemCurrent = listItemCurrent->next;
	}

	//[R]state change

	//user notify
	listItemCurrent = userServerList->head;
	while(listItemCurrent != NULL){
		//alter user character to be identifiable
		((UserServer*)listItemCurrent->data)->character->type = CharacterTypeYou;	
		networkSendClient(worldServer);
		((UserServer*)listItemCurrent->data)->character->type = CharacterTypeUser;

		listItemCurrent = listItemCurrent->next;
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
	userServerList = ListNew();

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
	WorldServerDelete(worldServer);

	//free userItemS
	ListDelete(userServerList, UserServerDelete);

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
	UserServer* userServer = Copy(&(UserServer){
		.auth = NULL,
		.character = NULL,
		.keyS = NULL,
		.keySLength = 0,
		.name = (char*) malloc((15 + 1) * sizeof(char)),
	}, sizeof(UserServer));
	strncpy(userServer->name, userServerUnsafe->name, 15);
	userServer->name[15] = '\0';
	ListInsert(&userServerList, userServer);

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
	//[R]
	Position positionCompressed = (Position){
		.y = 0,
		.x = 0,
	};
	Position position = {
		.y = 0,
		.x = 0,
	};
	List* collisionListObject = collisionObjectS(worldServer->objectList, position, position);
	List* collisionListCharacter = collisionCharacterS(worldServer->characterList, position, position);
	while(
		positionCompressed.y == 0 ||
		positionCompressed.x == 0 ||
		positionCompressed.y == worldServer->height - 1 ||
		positionCompressed.x == worldServer->width - 1 ||
		(
			positionCompressed.y % 2 == 0 &&
			positionCompressed.x % 2 == 0
		) ||
		collisionListObject->length != 0 ||
		collisionListCharacter->length != 0
	){
		positionCompressed.y = rand() % worldServer->height;
		positionCompressed.x = rand() % worldServer->width;

		position.y = positionCompressed.y * squaresize;
		position.x = positionCompressed.x * squaresize;

		free(collisionListObject);
		free(collisionListCharacter);
		collisionListObject = collisionObjectS(worldServer->objectList, position, position);
		collisionListCharacter = collisionCharacterS(worldServer->objectList, position, position);
	}

	//[R]clear area

	//character insert
	ListItem* listItemCharacter = ListInsert(&(worldServer->characterList), Copy(&(Character){
		.bombCount = 1,
		.name = userServer->name,
		.position = (Position) {
			position.y,
			position.x
		},
		.type = CharacterTypeUser,
		.velocity = velocity,
	}, sizeof(Character)));

	//character associate
	userServer->character = (Character*)listItemCharacter->data;

	//reply
	free(userServerUnsafe->auth); //in best case it's free(NULL)
	userServerUnsafe->auth = (char*) malloc((26 + 1) * sizeof(char));
	strcpy(userServerUnsafe->auth, userServer->auth);

	if(SDL_UnlockMutex(mutex) < 0){
		SDL_Log("ServerConnect: mutex unlock: %s", SDL_GetError());
		exit(1);
	}
}
