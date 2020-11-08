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

	//collision
	List* listCollisionObject = collisionObjectS(worldServer->objectList, userServer->character->position, positionNew);
	List* listCollisionCharacter = collisionCharacterS(worldServer->characterList, userServer->character->position, positionNew);

	//[R] collision should not drop position new, just cut it &positionNew to functions

	if (
		listCollisionCharacter->length != 1 ||
		listCollisionCharacter->head->data != userServer->character
	){
		ListDelete(listCollisionObject, NULL);
		ListDelete(listCollisionCharacter, NULL);
		return;
	}
	if(listCollisionObject->length != 0){
		for(ListItem* item = listCollisionObject->head; item != NULL; item = item->next){
			//player can be inside fire (it will die in this exact tick)
			if(((Object*)item->data)->type == ObjectTypeBombFire){
				continue;
			}

			//player can be inside bomb
			//it can be inside two bomb
			//eg: inside firstly placed one and just placed one into neighbourgh position
			if(
				((Object*)item->data)->type == ObjectTypeBomb &&
				((Object*)item->data)->owner == userServer->character &&
				!((Object*)item->data)->bombOut
			){
				continue;
			}

			ListDelete(listCollisionObject, NULL);
			ListDelete(listCollisionCharacter, NULL);
			return;
		}
	}

	//moved from an area with its own bombs
	if(listCollisionObject->length != 0){
		for(ListItem* item = listCollisionObject->head; item != NULL; item = item->next){
			//bombs which to player can not move back
			//(it can be that it moved out from it in the past)
			if(
				((Object*)item->data)->owner == userServer->character &&
				!collisionPoint(positionNew, ((Object*)item->data)->position
			)){
				((Object*)item->data)->bombOut = true;
			}
		}
	}
	
	userServer->character->position = positionNew;

	ListDelete(listCollisionObject, NULL);
	ListDelete(listCollisionCharacter, NULL);
}

//keyBomb calculates bomb position based on keyItem.key
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

	//collision
	List* collisionObjectList = collisionObjectS(worldServer->objectList, positionNew, positionNew);
	List* collisionCharacterList  = collisionCharacterS(worldServer->characterList, positionNew, positionNew);

	if (
		collisionCharacterList->length != 0 && (
			collisionCharacterList->length != 1 ||
			collisionCharacterList->head->data != userServer->character
		)
	){
		ListDelete(collisionObjectList, NULL);
		ListDelete(collisionCharacterList, NULL);
		return;
	}
	if(collisionObjectList->length != 0){
		ListDelete(collisionObjectList, NULL);
		ListDelete(collisionCharacterList, NULL);
		return;
	}
	ListDelete(collisionObjectList, NULL);
	ListDelete(collisionCharacterList, NULL);

	//bomb insert
	Object* object = ObjectNew();
	object->created = tickCount;
	object->destroy = tickCount + 2 * tickSecond;
	object->position = positionNew;
	object->type = ObjectTypeBomb;
	object->velocity = (Position){0, 0};
	object->bombOut = false;
	object->owner = userServer->character;
	ListInsert(&(worldServer->objectList), object);

	//bomb decrease
	userServer->character->bombCount--;
}

UserServer* ServerAuthCheck(char* auth){
	for(ListItem* item = userServerList->head; item != NULL; item = item->next){
		//connecting user
		if(((UserServer*)item->data)->auth == NULL){
			continue;
		}

		//timing attack safe compare
		bool diff = false;
		for(int i=0; i<26; i++){
			if(auth[i] != ((UserServer*)item->data)->auth[i]){
				diff = true;
			}
		}
		if(!diff){
			return (UserServer*)(item->data);
		}
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
			Object* objectFire = ObjectNew();
			objectFire->bombOut = true;
			objectFire->created = tickCount;
			objectFire->destroy = tickCount + 0.25 * tickSecond;
			objectFire->owner = object->owner;
			objectFire->position = (Position){
				.y = object->position.y + i * directionY[j] * squaresize,
				.x = object->position.x + i * directionX[j] * squaresize,
			};
			objectFire->type = ObjectTypeBombFire;
			objectFire->velocity = (Position) {
				.y = 0,
				.x = 0,
			};
			ListInsert(&(worldServer->objectList), objectFire);

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
	for(ListItem* item = listCollisionObject->head; item != NULL; item = item->next){
		if(((Object*)item->data)->type == ObjectTypeBox){
			ListItem* listItem = ListFindItem(worldServer->objectList, item->data);
			ListRemoveItem(&(worldServer->objectList), listItem, ObjectDelete);
		} else if(((Object*)item->data)->type == ObjectTypeBomb){
			//bombExplode(objectItemCurrent->object); [R]
		}
	}
	ListDelete(listCollisionObject, NULL);

	//character collision
	List* listCollisionCharacter = collisionCharacterS(worldServer->characterList, object->position, object->position);
	for(ListItem* item = listCollisionCharacter->head; item != NULL; item = item->next){
		ListItem* listItem = ListFindItem(worldServer->characterList, item->data);

		//make character dead
		((Character*)listItem->data)->owner->character = NULL;

		//remove item
		ListRemoveItem(&(worldServer->characterList), listItem, CharacterDelete);
	}
	ListDelete(listCollisionCharacter, NULL);
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
	for(ListItem* item = userServerList->head; item != NULL; item = item->next){
		if(((UserServer*)item->data)->character == NULL){
			continue;
		}

		for (int i=0; i<((UserServer*)item->data)->keySLength; i++){
			//should be before keyMovement as user wants to place bomb on the position currently seeable
			keyBomb(((UserServer*)item->data)->keyS[i], item->data);
			keyMovement(((UserServer*)item->data)->keyS[i], item->data);
		}
	}

	//destroy by user
	for(ListItem* item = worldServer->objectList->head; item != NULL; item = item->next){
		fireDestroy(item->data);
	}

	//[R]state change

	//user notify
	for(ListItem* item = userServerList->head; item != NULL; item = item->next){
		//alter user character to be identifiable
		if(((UserServer*)item->data)->character != NULL){
			((UserServer*)item->data)->character->type = CharacterTypeYou;	
		}
		networkSendClient(worldServer);
		if(((UserServer*)item->data)->character != NULL){
			((UserServer*)item->data)->character->type = CharacterTypeUser;	
		}
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
	if (mutex == NULL){
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
	int length = strnlen(userServerUnsafe->auth, 26 + 1); //[R] strnlen may overindex
	if(length != 26){
		if(SDL_UnlockMutex(mutex) < 0){
			SDL_Log("ServerReceive: mutex unlock: %s", SDL_GetError());
			exit(1);
		}
		return;
	}
	UserServer* userServer = ServerAuthCheck(userServerUnsafe->auth); //[R] auth may be shorther than 26
	if(userServer == NULL){
		if(SDL_UnlockMutex(mutex) < 0){
			SDL_Log("ServerReceive: mutex unlock: %s", SDL_GetError());
			exit(1);
		}
		return;
	}

	//alive
	if(userServer->character == NULL){
		if(SDL_UnlockMutex(mutex) < 0){
			SDL_Log("ServerReceive: mutex unlock: %s", SDL_GetError());
			exit(1);
		}
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
	if (SDL_LockMutex(mutex) != 0){
		SDL_Log("ServerConnect: SDL_LockMutex: %s", SDL_GetError());
		exit(1);
	}

	//userServer copy
	UserServer* userServer = UserServerNew();
	strncpy(userServer->name, userServerUnsafe->name, 15);
	userServer->name[15] = '\0';

	//userServer insert
	ListInsert(&userServerList, userServer);

	//id generate
	while (true){
		char* auth = ServerAuthCreate();
		
		//id exists
		if(ServerAuthCheck(auth) == NULL){
			free(userServer->auth);
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

		ListDelete(collisionListObject, NULL);
		ListDelete(collisionListCharacter, NULL);
		collisionListObject = collisionObjectS(worldServer->objectList, position, position);
		collisionListCharacter = collisionCharacterS(worldServer->objectList, position, position);
	}
	ListDelete(collisionListObject, NULL);
	ListDelete(collisionListCharacter, NULL);

	//[R]clear area to have a playable spawn

	//character insert
	Character* character = CharacterNew();
	character->bombCount = 1;
	character->owner = userServer;
	character->position = (Position) {
		position.y,
		position.x
	};
	character->type = CharacterTypeUser;
	character->velocity = velocity;
	ListInsert(&(worldServer->characterList), character);

	//character associate
	userServer->character = character;

	//reply
	strcpy(userServerUnsafe->auth, userServer->auth);

	if(SDL_UnlockMutex(mutex) < 0){
		SDL_Log("ServerConnect: mutex unlock: %s", SDL_GetError());
		exit(1);
	}
}
