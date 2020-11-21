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
#include "type/key.h"
#include "config.h"
#include <stdlib.h>

//mutex handles all global variable which is modified in critical sections
static SDL_mutex* mutex;
static List* userServerList;
static WorldServer* worldServer = NULL;
static long long tickCount = 0;
static int tickId;

//worldGenerate generates default map
//free should be called
void worldGenerate(int height, int width){
	if(height % 2 != 1 || width % 2 != 1){
		SDL_Log("worldGenerate: World size is malformed");
		exit(1);
	}

	if(worldServer != NULL){
		WorldServerDelete(worldServer);
	}
	worldServer = WorldServerNew();
	worldServer->height = height;
	worldServer->width = width;
	
	//wall generate
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

	int collisionFreeCountObject = CollisionFreeCountObjectGet(worldServer, (Position){
		.y = squaresize,
		.x = squaresize,
	});
	
	//box generate randomly
	for(int i=0; i < boxRatio * collisionFreeCountObject; i++){
		Object* object = ObjectNew();
		object->created = -1;
		object->destroy = -1;
		object->position = SpawnGet(worldServer, 1);
		object->type = ObjectTypeBox;
		object->velocity = (Position){
			.y = 0,
			.x = 0
		};
		object->bombOut = true;
		object->owner = NULL;
		ListInsert(&(worldServer->objectList), object);
	}

	//enemy generate randomly
	for(int i=0; i < enemyRatio * collisionFreeCountObject; i++){
		Character* character = CharacterNew();
		character->bombCount = 0;
		character->position = SpawnGet(worldServer, 1);
		character->type = CharacterTypeEnemy;
		character->velocity = velocityEnemy;
		character->keyS[rand() % KeyLength] = true;
		ListInsert(&(worldServer->characterList), character);
	}
}

UserServer* characterFindVariable;
bool characterFindFunction(void* data){
	return ((Character*)data)->owner == characterFindVariable;
}

//characterFind returns Character for UserServer
//can not be used in parallel
Character* characterFind(UserServer* userServer){
	characterFindVariable = userServer;
	ListItem* listItem = ListFindItemByFunction(worldServer->characterList, characterFindFunction);
	
	if(listItem == NULL){
		return NULL;
	}
	return listItem->data;
}

bool keyMovementCollisionDetectObject(void* this, Object* that){
	//player can be inside fire (it will die in this exact tick)
	if(that->type == ObjectTypeBombFire){
		return false;
	}

	//player can be inside bomb
	//it can be inside two bomb
	//eg: inside firstly placed one and just placed one into neighbourgh position
	if(
		that->type == ObjectTypeBomb &&
		that->owner == (Character*)this &&
		!that->bombOut
	){
		return false;
	}

	return true;
}

bool keyMovementCollisionDetectCharacter(void* this, Character* that){
	//CharacterTypeUser is solid for CharacterTypeUser
	//CharacterTypeEnemy is not solid for CharacterTypeUser
	//vice versa with CharacterTypeEnemy
	//so only same type character is solid
	if(that->type != ((Character*)this)->type){
		return false;
	}

	return true;
}

//keyMovement
//userServer must have a character
void keyMovement(Character* character){
	Position positionNew = character->position;
	if(character->keyS[KeyUp]){
		positionNew.y -= character->velocity;
	}
	if(character->keyS[KeyLeft]){
		positionNew.x -= character->velocity;
	}
	if(character->keyS[KeyDown]){
		positionNew.y += character->velocity;
	}
	if(character->keyS[KeyRight]){
		positionNew.x += character->velocity;
	}

	//collision
	positionNew = CollisionLinePositionGet(
		worldServer,
		character->position,
		positionNew,
		character,
		keyMovementCollisionDetectObject,
		keyMovementCollisionDetectCharacter
	);

	//enemy new one way direction
	if(
		character->type == CharacterTypeEnemy &&
		PositionSame(character->position, positionNew)
	){
		for(int i=0; i<KeyLength; i++){
			character->keyS[i] = false;
		}
		
		character->keyS[rand() % KeyLength] = true;
	}
	character->position = positionNew;

	//moved out from a bomb with !bombOut
	//in one move it is not possible that it moved out from bomb then moved back again
	for(ListItem* item = worldServer->objectList->head; item != NULL; item = item->next){
		Object* object = (Object*)item->data;
		if(
			object->type == ObjectTypeBomb &&
			object->owner == character &&
			!object->bombOut &&
			!CollisionPoint(character->position, object->position)
		){
			object->bombOut = true;
		}
	}
}

//keyBomb
void keyBomb(Character* character){
	if(!character->keyS[KeyBomb]){
		return;
	}

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
	List* collisionObjectS = CollisionPointAllObjectGet(worldServer->objectList, positionNew, NULL, NULL);
	List* collisionCharacterS = CollisionPointAllCharacterGet(worldServer->characterList, positionNew, character, NULL);

	if(
		collisionCharacterS->length != 0 ||
		collisionObjectS->length != 0
	){
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

UserServer* ServerAuthFind(char* auth){
	for(ListItem* item = userServerList->head; item != NULL; item = item->next){
		//connecting user
		//[R] this may not be happening because of mutex
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

//bombExplode removes bomb and creates fire in its place
//if object->type != ObjectTypeBomb then nothing happens
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
//if object->type != ObjectTypeFire then nothing happens
void fireDestroy(Object* object){
	if(object->type != ObjectTypeBombFire){
		return;
	}

	//object collision
	List* collisionObjectS = CollisionPointAllObjectGet(worldServer->objectList, object->position, NULL, NULL);
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
	List* collisionCharacterS = CollisionPointAllCharacterGet(worldServer->characterList, object->position, NULL, NULL);
	for(ListItem* item = collisionCharacterS->head; item != NULL; item = item->next){
		//remove item
		ListItem* listItem = ListFindItemByPointer(worldServer->characterList, item->data);
		ListRemoveItem(&(worldServer->characterList), listItem, CharacterDelete);
	}
	ListDelete(collisionCharacterS, NULL);
}

bool enemyKillCollisionDetectCharacter(void* this, Character* that){
	return that->type == CharacterTypeEnemy;
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

	//enemy random movement
	//must be before character movement as that fixes bumping into wall
	for(ListItem* item = worldServer->characterList->head; item != NULL; item = item->next){
		Character* character = (Character*)item->data;
		if(character->type != CharacterTypeEnemy){
			continue;
		}

		if(rand() % RAND_MAX + 1 > RAND_MAX * enemyKeyChangePossibility){
			continue;
		}

		//[R] key lib, random key function
		//[R] check if 10000*enemyKeyChangePossibility is not zero
		for(int i=0; i<KeyLength; i++){
			character->keyS[i] = false;
		}
		character->keyS[rand() % KeyLength] = true;
	}
	
	//character movement
	//this should be calculated before fireDestroy() otherwise player would be in fire for 1 tick
	//if 2 character is racing for the same spot the first in list wins
	for(ListItem* item = worldServer->characterList->head; item != NULL; item = item->next){
		keyBomb(item->data);
		keyMovement(item->data);
	}

	//destroy by user
	for(ListItem* item = worldServer->objectList->head; item != NULL; item = item->next){
		fireDestroy(item->data);
	}

	//destroy by enemy
	List* deathS = ListNew();
	for(ListItem* item = worldServer->characterList->head; item != NULL; item = item->next){
		Character* character = (Character*)item->data;
		if(character->type != CharacterTypeUser){
			continue;
		}

		List* collisionCharacterS = CollisionPointAllCharacterGet(
			worldServer->characterList,
			character->position,
			character,
			enemyKillCollisionDetectCharacter
		);	
		
		//death
		if(collisionCharacterS->length != 0){
			ListInsert(&deathS, character);
		}

		ListDelete(collisionCharacterS, NULL);
	}
	for(ListItem* item = deathS->head; item != NULL; item = item->next){
		ListItem* listItem = ListFindItemByPointer(worldServer->characterList, item->data);
		ListRemoveItem(&(worldServer->characterList), listItem, CharacterDelete);
	}
	ListDelete(deathS, NULL);

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
	worldGenerate(17, 57); //not critical section
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
	UserServer* userServer = ServerAuthFind(userServerUnsafe->auth); //[R] auth may be shorther than 26
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
	//[R] userServerUnsafe->keyS may not be KeyLength long
	for(int i=0; i<KeyLength; i++){
		character->keyS[i] = userServerUnsafe->keyS[i];
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
		if(ServerAuthFind(auth) == NULL){
			free(userServer->auth);
			userServer->auth = auth;
			break;
		}

		free(auth);
	}

	//spawn
	Position position = SpawnGet(worldServer, 3);

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
