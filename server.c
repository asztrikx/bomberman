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
#include "key.h"
#include <stdlib.h>
#include <stdio.h>

//mutex handles all global variable which is modified in critical sections
static SDL_mutex* mutex;
static List* userServerList;
static WorldServer* worldServer = NULL;
static long long tickCount = 0;
static int tickId;
static bool stopped = true;

//WorldGenerate generates default map
static void WorldGenerate(int height, int width){
	if(
		height % 2 != 1 ||
		width % 2 != 1 ||
		height < 5 ||
		width < 5
	){
		SDL_Log("WorldGenerate: World size is malformed");
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
				ListInsert(&(worldServer->objectList), object);
			}
		}
	}

	int collisionFreeCountObject = CollisionFreeCountObjectGet(worldServer, (Position){
		.y = squaresize,
		.x = squaresize,
	});
	
	//box generate randomly
	for(int i=0; i < (int)(boxRatio * collisionFreeCountObject); i++){
		Object* object = ObjectNew();
		object->position = SpawnGet(worldServer, 1);
		object->type = ObjectTypeBox;
		ListInsert(&(worldServer->objectList), object);
	}

	//exit
	Object* object = ObjectNew();
	object->position = ((Object*)worldServer->objectList->head->data)->position;
	object->type = ObjectTypeExit;
	ListInsert(&(worldServer->objectList), object);
	worldServer->exit = object;

	//enemy generate randomly
	for(int i=0; i < (int)(enemyRatio * collisionFreeCountObject); i++){
		Character* character = CharacterNew();
		character->position = SpawnGet(worldServer, 3);
		character->type = CharacterTypeEnemy;
		character->velocity = velocityEnemy;
		KeyMovementRandom(character);
		ListInsert(&(worldServer->characterList), character);
	}
}

UserServer* characterFindVariable;

//CharacterFindFunction is a helper function of CharacterFind
static bool CharacterFindFunction(void* data){
	return ((Character*)data)->owner == characterFindVariable;
}

//CharacterFind returns Character for UserServer
//can not be used in parallel
static Character* CharacterFind(UserServer* userServer){
	characterFindVariable = userServer;
	ListItem* listItem = ListFindItemByFunction(worldServer->characterList, CharacterFindFunction);
	
	if(listItem == NULL){
		return NULL;
	}
	return listItem->data;
}

//AuthFind returns UserServer with that auth or NULL if does not exists
static UserServer* AuthFind(char* auth){
	for(ListItem* item = userServerList->head; item != NULL; item = item->next){
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

//AuthCreate creates a 26 character long auth key
static char* AuthCreate(){
	char* auth = (char*) malloc((26 + 1) * sizeof(char));
	for(int i=0; i<26; i++){
		auth[i] = rand() % ('Z' - 'A' + 1) + 'A';
	}
	auth[26] = '\0';

	return auth;
}

//TickCalculateDestroyBomb removes bomb and creates fire in its place
//if object->type != ObjectTypeBomb then nothing happens
static void TickCalculateDestroyBomb(Object* object){
	//fire inserts
	int directionX[] = {0, 1, -1, 0, 0};
	int directionY[] = {0, 0, 0, 1, -1};
	for(int j=0; j<5; j++){
		Position position = (Position){
			.y = object->position.y + directionY[j] * squaresize,
			.x = object->position.x + directionX[j] * squaresize,
		};

		List* collisionObjectS = CollisionObjectSGet(worldServer->objectList, position, object, NULL);
		bool boxExists = collisionObjectS->length == 0;
		for(ListItem* item = collisionObjectS->head; item != NULL; item = item->next){
			if(((Object*)item->data)->type == ObjectTypeBox){
				boxExists = true;
				break;
			}
		}
		if(!boxExists && collisionObjectS->length != 0){
			ListDelete(collisionObjectS, NULL);
			continue;
		}
		ListDelete(collisionObjectS, NULL);

		Object* objectFire = ObjectNew();
		objectFire->bombOut = true;
		objectFire->created = tickCount;
		objectFire->destroy = tickCount + 0.25 * tickSecond;
		objectFire->owner = object->owner;
		objectFire->position = position;
		objectFire->type = ObjectTypeBombFire;
		objectFire->velocity = 0;

		ListInsert(&(worldServer->objectList), objectFire);
	}

	//bomb remove
	if(object->owner != NULL){
		object->owner->bombCount++;
	}
}

//TickCalculateFireDestroy makes fires destroys all ObjectTypeBox and all Character in collision
static void TickCalculateFireDestroy(){
	for(ListItem* item = worldServer->objectList->head; item != NULL; item = item->next){
		Object* object = item->data;
		if(object->type != ObjectTypeBombFire){
			continue;
		}
		
		//object collision
		List* collisionObjectS = CollisionObjectSGet(worldServer->objectList, object->position, NULL, NULL);
		for(ListItem* item = collisionObjectS->head; item != NULL; item = item->next){
			if(((Object*)item->data)->type == ObjectTypeBox){
				ListItem* listItem = ListFindItemByPointer(worldServer->objectList, item->data);
				ListRemoveItem(&(worldServer->objectList), listItem, ObjectDelete);
			} else if(((Object*)item->data)->type == ObjectTypeBomb){
				//chain bomb explosion
				//-
				//bombExplode(objectItemCurrent->object);
			}
		}
		ListDelete(collisionObjectS, NULL);

		//character collision
		List* collisionCharacterS = CollisionCharacterSGet(worldServer->characterList, object->position, NULL, NULL);
		for(ListItem* item = collisionCharacterS->head; item != NULL; item = item->next){
			//get
			ListItem* listItem = ListFindItemByPointer(worldServer->characterList, item->data);

			//UserServer update
			if(((Character*)listItem->data)->owner != NULL){
				((Character*)listItem->data)->owner->gamestate = GamestateDead;
			}

			//remove
			ListRemoveItem(&(worldServer->characterList), listItem, CharacterDelete);
		}
		ListDelete(collisionCharacterS, NULL);
	}
}

//TickCalculateEnemyKillCollisionDetect is a helper function of TickCalculateEnemyKill
static bool TickCalculateEnemyKillCollisionDetect(void* this, Character* that){
	return that->type == CharacterTypeEnemy;
}

//TickCalculateWin checks if any CharacterTypeUser if in a winning state and removes them if so
static void TickCalculateWin(){
	List* collisionCharacterS = CollisionCharacterSGet(worldServer->characterList, worldServer->exit->position, NULL, NULL);
	for(ListItem* item = collisionCharacterS->head; item != NULL; item = item->next){
		Character* character = item->data;
		if(
			character->type == CharacterTypeUser &&
			worldServer->characterList->length == 1
		){
			//UserServer update
			character->owner->gamestate = GamestateWon;

			//remove
			ListItem* listItem = ListFindItemByPointer(worldServer->characterList, character);
			ListRemoveItem(&(worldServer->characterList), listItem, CharacterDelete);
		}
	}
	ListDelete(collisionCharacterS, NULL);
}

//TickCalculateEnemyKill checks if any CharacterTypeUser is colliding with CharacterTypeEnemy and kills them if so
static void TickCalculateEnemyKill(){
	List* deathS = ListNew();
	for(ListItem* item = worldServer->characterList->head; item != NULL; item = item->next){
		Character* character = item->data;
		if(character->type != CharacterTypeUser){
			continue;
		}

		List* collisionCharacterS = CollisionCharacterSGet(
			worldServer->characterList,
			character->position,
			character,
			TickCalculateEnemyKillCollisionDetect
		);

		//death
		if(collisionCharacterS->length != 0){
			character->owner->gamestate = GamestateDead;
			ListInsert(&deathS, item);
		}

		ListDelete(collisionCharacterS, NULL);
	}
	for(ListItem* item = deathS->head; item != NULL; item = item->next){
		ListRemoveItem(&(worldServer->characterList), (ListItem*)item->data, CharacterDelete);
	}
	ListDelete(deathS, NULL);
}

//TickCalculateEnemyMovement randomly creates a new random direction for CharacterTypeEnemys
static void TickCalculateEnemyMovement(){
	for(ListItem* item = worldServer->characterList->head; item != NULL; item = item->next){
		Character* character = item->data;
		if(character->type != CharacterTypeEnemy){
			continue;
		}

		if(rand() % RAND_MAX + 1 > RAND_MAX * enemyKeyChangePossibility){
			continue;
		}

		KeyMovementRandom(character);
	}
}

//TickCalculateDestroy removes items where .destroy == tickCount
//destroy hooks also added here
static void TickCalculateDestroy(){
	ListItem* listItemCurrent = worldServer->objectList->head;
	while(listItemCurrent != NULL){
		if(tickCount != ((Object*)listItemCurrent->data)->destroy){
			listItemCurrent = listItemCurrent->next;
			continue;
		}

		if(((Object*)listItemCurrent->data)->type == ObjectTypeBomb){
			TickCalculateDestroyBomb(listItemCurrent->data);
		}

		listItemCurrent = listItemCurrent->next;

		ListRemoveItem(&(worldServer->objectList), listItemCurrent->prev, ObjectDelete);
	}
}

//TickCalculateAnimate calculates next texture state from current
static void TickCalculateAnimate(){
	//animate
	for (ListItem* item = worldServer->objectList->head; item != NULL; item = item->next){
		Object* object = item->data;

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
		Character* character = item->data;

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

//TickCalculate calculates next state from current
static void TickCalculate(){
	//this should be calculated first as these objects should not exists in this tick
	TickCalculateDestroy();

	//must be before character movement as that fixes bumping into wall
	TickCalculateEnemyMovement();
	
	//character movement
	//this should be calculated before TickCalculateFireDestroy() otherwise player would be in fire for 1 tick
	//if 2 character is racing for the same spot the first in list wins
	for(ListItem* item = worldServer->characterList->head; item != NULL; item = item->next){
		if(((Character*)item->data)->keyS[KeyBomb]){
			KeyBombPlace(item->data, worldServer, tickCount);
		}
		KeyMovement(item->data, worldServer);
	}

	//should be before any destroy
	TickCalculateWin();

	TickCalculateFireDestroy();

	TickCalculateEnemyKill();

	TickCalculateAnimate();
}

//TickSend sends new world to connected clients
static void TickSend(){
	for(ListItem* item = userServerList->head; item != NULL; item = item->next){
		//remove exit if not seeable
		List* collisionObjectS = CollisionObjectSGet(worldServer->objectList, worldServer->exit->position, worldServer->exit, NULL);
		Object* exit = worldServer->exit;
		if(collisionObjectS->length != 0){
			//NetworkSendClient will remove it from list
			worldServer->exit = NULL;
		}
		ListDelete(collisionObjectS, NULL);
		
		//alter user character to be identifiable
		Character* character = CharacterFind((UserServer*)item->data);
		if(character != NULL){
			character->type = CharacterTypeYou;	
		}

		//send
		NetworkSendClient(worldServer, (UserServer*)item->data);

		//remove character alter
		if(character != NULL){
			character->type = CharacterTypeUser;	
		}

		//remove exit remove
		worldServer->exit = exit;
	}
}

//Tick calculates new frame, notifies users
Uint32 Tick(Uint32 interval, void *param){
	if(SDL_LockMutex(mutex) != 0){
		SDL_Log("Tick: SDL_LockMutex: %s", SDL_GetError());
		exit(1);
	}
	
	TickCalculate();

	TickSend();

	if(SDL_UnlockMutex(mutex) < 0){
		SDL_Log("Tick: mutex unlock: %s", SDL_GetError());
		exit(1);
	}

	tickCount++;
	return interval;
}

//Save saves worldServer and tickCount into world.save
void Save(){
	FILE* file = fopen("world.save", "wt");
	if(file == NULL){
		SDL_Log("Save: (Over)Write file");
		exit(1);
	}

	//height
	fprintf(file, "%d\n", worldServer->height);

	//width
	fprintf(file, "%d\n", worldServer->width);

	//tickCount
	fprintf(file, "%lld\n", tickCount);

	//worldServer->characterList->length
	fprintf(file, "%d\n", worldServer->characterList->length);

	//worldServer->characterList
	for(ListItem* item = worldServer->characterList->head; item != NULL; item = item->next){
		Character* character = item->data;

		//do not save things not handled by server
		if(character->owner != NULL){
			continue;
		}

		fprintf(file, "%d\n", character->position.y);
		fprintf(file, "%d\n", character->position.x);
		fprintf(file, "%d\n", character->type);
		fprintf(file, "%d\n", character->velocity);
		fprintf(file, "%d\n", character->bombCount);

		//no use saving these
		//fprintf(file, "", character->animation);
		//fprintf(file, "", character->keyS);
	}

	//worldServer->objectList->length
	fprintf(file, "%d\n", worldServer->objectList->length);

	//worldServer->objectList
	for(ListItem* item = worldServer->objectList->head; item != NULL; item = item->next){
		Object* object = item->data;

		//do not save things not handled by server
		if(object->owner != NULL){
			continue;
		}

		fprintf(file, "%d\n", object->position.y);
		fprintf(file, "%d\n", object->position.x);
		fprintf(file, "%d\n", object->type);
		fprintf(file, "%lld\n", object->created);
		fprintf(file, "%lld\n", object->destroy);
		fprintf(file, "%d\n", object->velocity);
		fprintf(file, "%d\n", object->bombOut ? 1 : 0);

		//no use saving these
		//fprintf(file, """, object->animation);
	}

	fclose(file);
}

//Load loads world.save into worldServer
void Load(){
	if(worldServer != NULL){
		WorldServerDelete(worldServer);
	}
	worldServer = WorldServerNew();

	FILE* file = fopen("world.save", "rt");
	if(file == NULL){
		SDL_Log("Load: Open file");
		exit(1);
	}

	//height
	fscanf(file, "%d\n", &worldServer->height);

	//width
	fscanf(file, "%d\n", &worldServer->width);

	//tickCount
	fscanf(file, "%lld\n", &tickCount);

	//worldServer->characterList->length
	int characterListLength;
	fscanf(file, "%d\n", &characterListLength);

	//worldServer->characterList
	for(int i=0; i < characterListLength; i++){
		Character* character = CharacterNew();

		fscanf(file, "%d\n", &character->position.y);
		fscanf(file, "%d\n", &character->position.x);
		int characterType;
		fscanf(file, "%d\n", &characterType);
		character->type = (CharacterType)characterType;
		fscanf(file, "%d\n", &character->velocity);
		fscanf(file, "%d\n", &character->bombCount);

		//not saved
		//fscanf(file, "", &character->animation);
		//fscanf(file, "", &character->keyS);

		ListInsert(&worldServer->characterList, character);
	}

	//worldServer->objectList->length
	int objectListLength;
	fscanf(file, "%d\n", &objectListLength);

	//worldServer->objectList
	for(int i=0; i < objectListLength; i++){
		Object* object = ObjectNew();

		fscanf(file, "%d\n", &object->position.y);
		fscanf(file, "%d\n", &object->position.x);
		int objectType;
		fscanf(file, "%d\n", &objectType);
		object->type = (ObjectType)objectType;
		fscanf(file, "%lld\n", &object->created);
		fscanf(file, "%lld\n", &object->destroy);
		fscanf(file, "%d\n", &object->velocity);
		int bombout;
		fscanf(file, "%d\n", &bombout);
		object->bombOut = bombout == 1;

		//not saved
		//fscanf(file, """, &object->animation);

		ListInsert(&worldServer->objectList, object);

		//exit set
		if(object->type == ObjectTypeExit){
			worldServer->exit = object;
		}
	}

	fclose(file);
}

//EventKey handles WorldServer saving
static int EventKey(void* data, SDL_Event* sdl_event){
	if(
		sdl_event->type != SDL_KEYDOWN ||
		sdl_event->key.keysym.sym != SDLK_q
	){
		return 0;
	}

	if (SDL_LockMutex(mutex) != 0){
		SDL_Log("EventKey: SDL_LockMutex: %s", SDL_GetError());
		exit(1);
	}

	Save();

	if(SDL_UnlockMutex(mutex) < 0){
		SDL_Log("EventKey: mutex unlock: %s", SDL_GetError());
		exit(1);
	}

	return 0;
}

//ServerStart generates world, start accepting connections, starts ticking
void ServerStart(bool load){
	stopped = false;

	//world set
	if(!load){
		WorldGenerate(worldHeight, worldWidth); //not critical section
	} else {
		Load();
	}

	userServerList = ListNew();

	//mutex init
	mutex = SDL_CreateMutex();
	if (mutex == NULL){
		SDL_Log("ServerStart: mutex create: %s", SDL_GetError());
		exit(1);
	}

	//key press
	SDL_AddEventWatch(EventKey, NULL);

	//network start
	NetworkServerStart();

	//tick start: world calc, connected user update
	tickId = SDL_AddTimer(tickRate, Tick, NULL);
	if (tickId == 0){
		SDL_Log("SDL_AddTimer: %s", SDL_GetError());
		exit(1);
	}
}

//ServerReceive gets updates from users
//userServerUnsafe is not used after return
void ServerReceive(UserServer* userServerUnsafe){
	if(stopped){
		return;
	}

	if (SDL_LockMutex(mutex) != 0){
		SDL_Log("ServerReceive: SDL_LockMutex: %s", SDL_GetError());
		exit(1);
	}

	//auth validate
	//auth's length validation
	//-
	int length = strnlen(userServerUnsafe->auth, 26 + 1);
	if(length != 26){
		if(SDL_UnlockMutex(mutex) < 0){
			SDL_Log("ServerReceive: mutex unlock: %s", SDL_GetError());
			exit(1);
		}
		return;
	}
	//auth's length validation
	//-
	UserServer* userServer = AuthFind(userServerUnsafe->auth);
	if(userServer == NULL){
		if(SDL_UnlockMutex(mutex) < 0){
			SDL_Log("ServerReceive: mutex unlock: %s", SDL_GetError());
			exit(1);
		}
		return;
	}
	Character* character = CharacterFind(userServer);

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

	//keyS's length validation
	//-

	//keyS copy
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

	//need to be called before NetworkServerStop as incoming message may already be coming which
	//could get stuck if SDL_DestroyMutex happens before SDL_LockMutex
	stopped = true;

	NetworkServerStop();

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
	userServer->gamestate = GamestateRunning;

	//userServer insert
	ListInsert(&userServerList, userServer);

	//id generate
	while (true){
		char* auth = AuthCreate();
		
		//id exists
		if(AuthFind(auth) == NULL){
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
