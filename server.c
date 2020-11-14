#include "debugmalloc.h"
#include "server.h"
#include <stdbool.h>
#include "geometry.h"
#include "SDL.h"
#include "network.h"
#include "type/object.h"
#include "type/user/server.h"
#include "type/world/server.h"
#include "type/character.h"
#include "config.h"

//mutex handles all global variable which is modified in critical sections
static SDL_mutex* mutex;
static List* userServerList;
static WorldServer* worldServer;
static long long tickCount = 0;
static int tickId;

//worldGenerate generates default map
//free should be called
WorldServer* worldGenerate(int height, int width, double boxRatio){
	if(height % 2 != 1 || width % 2 != 1){
		SDL_Log("worldGenerate: World size is malformed");
		exit(1);
	}

	WorldServer* worldServer = WorldServerNew();
	worldServer->height = height;
	worldServer->width = width;
	
	for(int i=0; i<height; i++){
		for(int j=0; j<width; j++){
			if(
				i == 0 || j == 0 ||
				i == height - 1 || j == width - 1 ||
				(i % 2 == 0 && j % 2 == 0)
			){
				Object* object = ObjectNew();
				object->position = (Position){
					.y = i * squaresize,
					.x = j * squaresize,
				};
				object->type = ObjectTypeWall;
				object->bombOut = true;
				ListInsert(&(worldServer->objectList), object);
			}
		}
	}
	
	//box generate
	if(
		RAND_MAX != INT32_MAX && (
			RAND_MAX + 1 < height ||
			RAND_MAX + 1 < width
		)
	){
		SDL_Log("worldGenerate: map is too big");
		exit(1);
	}
	int boxCount = boxRatio * CollisionFreeCountObjectGet(worldServer, (Position){
		.y = squaresize,
		.x = squaresize,
	});
	for(int i=0; i<boxCount; i++){
		//position
		Position position;
		int collisionCount;
		do{
			//random
			position.y = (rand() % height) * squaresize;
			position.x = (rand() % width) * squaresize;

			//collision
			List* collisionObjectS = CollisionObjectSGet(worldServer->objectList, position, position);
			collisionCount = collisionObjectS->length;
			ListDelete(collisionObjectS, NULL);
		} while (collisionCount != 0);

		Object* object = ObjectNew();
		object->created = -1;
		object->destroy = -1;
		object->position = position;
		object->type = ObjectTypeBox;
		object->velocity = (Position){
			.y = 0,
			.x = 0
		};
		object->bombOut = true;
		object->owner = NULL;
		ListInsert(&(worldServer->objectList), object);
	}

	return worldServer;
}

UserServer* listFindByFunctionCharacterOwnerVariable;
bool listFindByFunctionCharacterOwnerFunction(void* data){
	return ((Character*)data)->owner == listFindByFunctionCharacterOwnerVariable;
}

//characterFind returns Character for UserServer
//can not be used in parallel
Character* characterFind(UserServer* userServer){
	listFindByFunctionCharacterOwnerVariable = userServer;
	ListItem* listItem = ListFindItemByFunction(worldServer->characterList, listFindByFunctionCharacterOwnerFunction);
	
	if(listItem == NULL){
		return NULL;
	}
	return listItem->data;
}

//keyMovement calculates user position based on keyItem.key
//userServer must have a character
void keyMovement(SDL_Keycode key, UserServer* userServer){
	if(
		key != SDLK_w &&
		key != SDLK_a &&
		key != SDLK_s &&
		key != SDLK_d
	){
		return;
	}

	Character* character = characterFind(userServer);

	Position positionNew = character->position;
	if(key == SDLK_w){
		positionNew.y -= character->velocity.y;
	} else if(key == SDLK_a){
		positionNew.x -= character->velocity.x;
	} else if(key == SDLK_s){
		positionNew.y += character->velocity.y;
	} else if(key == SDLK_d){
		positionNew.x += character->velocity.x;
	}

	//collision
	List* collisionObjectS = CollisionObjectSGet(worldServer->objectList, character->position, positionNew);
	List* collisionCharacterS = CollisionCharacterSGet(worldServer->characterList, character->position, positionNew);

	//[R] collision should not drop position new, just cut it &positionNew to functions

	if (
		collisionCharacterS->length != 1 ||
		collisionCharacterS->head->data != character
	){
		ListDelete(collisionObjectS, NULL);
		ListDelete(collisionCharacterS, NULL);
		return;
	}
	if(collisionObjectS->length != 0){
		for(ListItem* item = collisionObjectS->head; item != NULL; item = item->next){
			//player can be inside fire (it will die in this exact tick)
			if(((Object*)item->data)->type == ObjectTypeBombFire){
				continue;
			}

			//player can be inside bomb
			//it can be inside two bomb
			//eg: inside firstly placed one and just placed one into neighbourgh position
			if(
				((Object*)item->data)->type == ObjectTypeBomb &&
				((Object*)item->data)->owner == character &&
				!((Object*)item->data)->bombOut
			){
				continue;
			}

			ListDelete(collisionObjectS, NULL);
			ListDelete(collisionCharacterS, NULL);
			return;
		}
	}

	//moved from an area with its own bombs
	if(collisionObjectS->length != 0){
		for(ListItem* item = collisionObjectS->head; item != NULL; item = item->next){
			//bombs which to player can not move back
			//(it can be that it moved out from it in the past)
			if(
				((Object*)item->data)->owner == character &&
				!CollisionPointGet(positionNew, ((Object*)item->data)->position
			)){
				((Object*)item->data)->bombOut = true;
			}
		}
	}
	
	character->position = positionNew;

	ListDelete(collisionObjectS, NULL);
	ListDelete(collisionCharacterS, NULL);
}

//keyBomb calculates bomb position based on keyItem.key
void keyBomb(SDL_Keycode key, UserServer* userServer){
	if(key != SDLK_SPACE){
		return;
	}

	Character* character = characterFind(userServer);

	//bomb available
	if(character->bombCount == 0){
		return;
	}

	Position positionNew = character->position;

	//position
	positionNew.y -= positionNew.y % squaresize;
	positionNew.x -= positionNew.x % squaresize;
	if (character->position.y % squaresize > squaresize / 2){
		positionNew.y += squaresize;
	}
	if (character->position.x % squaresize > squaresize / 2){
		positionNew.x += squaresize;
	}

	//collision
	List* collisionObjectS = CollisionObjectSGet(worldServer->objectList, positionNew, positionNew);
	List* collisionCharacterS = CollisionCharacterSGet(worldServer->characterList, positionNew, positionNew);

	if (
		collisionCharacterS->length != 0 && (
			collisionCharacterS->length != 1 ||
			collisionCharacterS->head->data != character
		)
	){
		ListDelete(collisionObjectS, NULL);
		ListDelete(collisionCharacterS, NULL);
		return;
	}
	if(collisionObjectS->length != 0){
		ListDelete(collisionObjectS, NULL);
		ListDelete(collisionCharacterS, NULL);
		return;
	}
	ListDelete(collisionObjectS, NULL);
	ListDelete(collisionCharacterS, NULL);

	//bomb insert
	Object* object = ObjectNew();
	object->created = tickCount;
	object->destroy = tickCount + 2 * tickSecond;
	object->position = positionNew;
	object->type = ObjectTypeBomb;
	object->velocity = (Position){0, 0};
	object->bombOut = false;
	object->owner = character;
	ListInsert(&(worldServer->objectList), object);

	//bomb decrease
	character->bombCount--;
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

//clientDrawCharacterFind destroys all ObjectTypeBox and ObjectTypeCharacter if object is ObjectTypeBombFire
void fireDestroy(Object* object){
	if(object->type != ObjectTypeBombFire){
		return;
	}

	//object collision
	List* collisionObjectS = CollisionObjectSGet(worldServer->objectList, object->position, object->position);
	for(ListItem* item = collisionObjectS->head; item != NULL; item = item->next){
		if(((Object*)item->data)->type == ObjectTypeBox){
			ListItem* listItem = ListFindItemByPointer(worldServer->objectList, item->data);
			ListRemoveItem(&(worldServer->objectList), listItem, ObjectDelete);
		} else if(((Object*)item->data)->type == ObjectTypeBomb){
			//bombExplode(objectItemCurrent->object); [R]
		}
	}
	ListDelete(collisionObjectS, NULL);

	//character collision
	List* collisionCharacterS = CollisionCharacterSGet(worldServer->characterList, object->position, object->position);
	for(ListItem* item = collisionCharacterS->head; item != NULL; item = item->next){
		//remove item
		ListItem* listItem = ListFindItemByPointer(worldServer->characterList, item->data);
		ListRemoveItem(&(worldServer->characterList), listItem, CharacterDelete);
	}
	ListDelete(collisionCharacterS, NULL);
}

//serverTickCalculate calculates new world state from current
void serverTickCalculate(){
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
		Character* character = characterFind((UserServer*)item->data);
		if(character == NULL){
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

	//animate
	for (ListItem* item = worldServer->objectList->head; item != NULL; item = item->next){
		Object* object = (Object*)item->data;

		//delay
		object->animation.stateDelayTick++;
		if(object->animation.stateDelayTick <= object->animation.stateDelayTickEnd){
			continue;
		}
		object->animation.stateDelayTick = 0;

		//state next
		object->animation.state++;
		object->animation.state %= TextureSSObject[object->type]->length;
	}
	for (ListItem* item = worldServer->characterList->head; item != NULL; item = item->next){
		Character* character = (Character*)item->data;

		//delay
		character->animation.stateDelayTick++;
		if(character->animation.stateDelayTick <= character->animation.stateDelayTickEnd){
			continue;
		}
		character->animation.stateDelayTick = 0;

		//state next
		character->animation.state++;
		character->animation.state %= TextureSSCharacter[character->type]->length;
	}
}

//serverTickSend sends new world to connected clients
void serverTickSend(){
	for(ListItem* item = userServerList->head; item != NULL; item = item->next){
		Character* character = characterFind((UserServer*)item->data);

		//alter user character to be identifiable
		if(character != NULL){
			character->type = CharacterTypeYou;	
		}
		networkSendClient(worldServer);
		if(character != NULL){
			character->type = CharacterTypeUser;	
		}
	}
}

//serverTick calculates new frame, notifies users
Uint32 serverTick(Uint32 interval, void *param){
	if(SDL_LockMutex(mutex) != 0){
		SDL_Log("serverTick: SDL_LockMutex: %s", SDL_GetError());
		exit(1);
	}
	
	serverTickCalculate();

	serverTickSend();

	if(SDL_UnlockMutex(mutex) < 0){
		SDL_Log("serverTick: mutex unlock: %s", SDL_GetError());
		exit(1);
	}

	tickCount++;
	return interval;
}

//ServerStart generates world, start accepting connections, starts ticking
void ServerStart(void){
	//world generate
	worldServer = worldGenerate(17, 57, 0.4); //not critical section
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
	tickId = SDL_AddTimer(tickRate, serverTick, NULL);
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
	Character* character = characterFind(userServer);

	//alive
	if(character == NULL){
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

	networkServerStop(); //[R] should be a mutex in network otherwise a lock can be stuck if real network is used

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

	//spawn
	Position position = SpawnGet(worldServer);

	//character insert
	Character* character = CharacterNew();
	character->bombCount = 1;
	character->owner = userServer;
	character->position = (Position) {
		.y = position.y,
		.x = position.x
	};
	character->type = CharacterTypeUser;
	character->velocity = velocity;
	ListInsert(&(worldServer->characterList), character);

	//reply
	strcpy(userServerUnsafe->auth, userServer->auth);

	if(SDL_UnlockMutex(mutex) < 0){
		SDL_Log("ServerConnect: mutex unlock: %s", SDL_GetError());
		exit(1);
	}
}
