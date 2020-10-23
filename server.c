#include "debugmalloc.h"
#include "server.h"
#include <stdbool.h>
#include "state.h"
#include "network.h"

static UserServerItem* userServerItemS;
static WorldServer* worldServer;
static long long tickCount = 0;
static unsigned int tickRate = 60u;

//[R] move to geom
void ServerWorldGenerate(int height, int width){
	worldServer = (WorldServer*) malloc(sizeof(WorldServer));
	worldServer->objectItemS = NULL;
	worldServer->characterItemS = NULL;
	worldServer->exit = NULL;
	
	for(int i=0; i<height; i++){
		int y = i * squaresize;
		int x = 0;

		//objectItem
		ObjectItem* objectItem = (ObjectItem*) malloc(sizeof(ObjectItem));
		objectItem->object = (Object){
			.created = tickCount,
			.position = (Position){
				.y = y,
				.x = x,
			},
			.type = ObjectTypeWall,
			.velocity = (Position){0,0},
		};
		
		//objectItemS first element
		if(worldServer->objectItemS == NULL){
			objectItem->next = NULL;
			objectItem->prev = NULL;

			worldServer->objectItemS = objectItem;

			continue;
		}

		//objectItemS insert front
		objectItem->next = worldServer->objectItemS;
		objectItem->prev = NULL;

		worldServer->objectItemS->prev = objectItem;

		worldServer->objectItemS = objectItem;
	}
}

//ServerTick calculates new frame, notifies users
Uint32 ServerTick(Uint32 interval, void *param){
	//[R]bomb explosion
	//[R]player death
	//[R]crate delete

	//user notify
	UserServerItem* userServerItemCurrent = userServerItemS;
	while(userServerItemCurrent != NULL){
		networkSendClient(worldServer);

		userServerItemCurrent = userServerItemCurrent->next;
	}

	return interval;
}

//ServerStart generates world, start ticking
void ServerStart(void){
	ServerWorldGenerate(10, 10);

	SDL_AddTimer(1000u/tickRate, ServerTick, NULL);
}
//[R] move to geom
bool collisionPositionS(Position position1, Position position2){
	if(abs(position1.x - position2.x) >= squaresize){
		return false;
	}
	if(abs(position1.y - position2.y) >= squaresize){
		return false;
	}
	return true;
}
//[R] move to geom
bool collisionObjectS(Position position){
	ObjectItem* objectItem = worldServer->objectItemS;
	while(objectItem != NULL){
		if(collisionPositionS(objectItem->object.position, position)){
			return true;
		}

		objectItem = objectItem->next;
	}
	
	return false;
}
//[R] move to geom
bool collisionCharacterS(Position position, Character* characterException){
	CharacterItem* characterItem = worldServer->characterItemS;
	while(characterItem != NULL){
		if(
			collisionPositionS(characterItem->character.position, position) &&
			&(characterItem->character) != characterException
		){
			return true;
		}

		characterItem = characterItem->next;
	}
	
	return false;
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
		collisionObjectS(positionNew) ||
		collisionCharacterS(positionNew, userServer->character)
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
	return;

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
		collisionObjectS(positionNew) ||
		collisionCharacterS(positionNew, userServer->character)
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
	//auth validate
	UserServerItem* userServerItemCurrent = userServerItemS;
	while(userServerItemCurrent != NULL){
		//timing attack safe compare
		bool diff = false;
		for(int i=0; i<26; i++){
			if(userServerUnsafe->auth[i] != userServerItemCurrent->userServer.auth[i]){
				diff = true;
			}
		}
		if(!diff){
			break;
		}

		userServerItemCurrent = userServerItemCurrent->next;
	}

	if(userServerItemCurrent == NULL){
		return;
	}
	UserServer* userServer = &(userServerItemCurrent->userServer);

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
}

//ServerStop clears server module
void ServerStop(void){
	//exit
	free(worldServer->exit);
	
	//characterItems
	CharacterItem* characterItemCurrent = worldServer->characterItemS;
	CharacterItem* characterItemPrev;
	while(characterItemCurrent != NULL){
		characterItemPrev = characterItemCurrent;
		characterItemCurrent = characterItemCurrent->next;

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

		free(userServerItemPrev);
	}
}

//ServerConnect register new connection user, returns it with auth
void ServerConnect(UserServer* userServerUnsafe){
	//[R]check if game is running

	//copy
	UserServerItem* userServerItem = (UserServerItem*) malloc(sizeof(UserServerItem));
	UserServer* userServer = &(userServerItem->userServer);
	userServer->auth = (char*) malloc((26 + 1) * sizeof(char)); //30years to crack
	userServer->character = NULL;
	userServer->keyS = NULL;
	userServer->keySLength = 0;
	userServer->name = (char*) malloc((15 + 1) * sizeof(char));
	strncpy(userServer->name, userServerUnsafe->name, 15);
	userServer->name[16] = '\0';

	//id generate
	while (true){
		for(int i=0; i<26; i++){
			userServer->auth[i] = rand() % ('Z' - 'A' + 1) + 'A';
		}
		userServer->auth[26] = '\0';
		
		UserServerItem* userServerItemCurrent = userServerItemS;
		while(userServerItemCurrent != NULL){
			if(strcmp(userServerItemCurrent->userServer.auth, userServer->auth) == 0){
				break;
			}		
			userServerItemCurrent = userServerItemCurrent->next;
		}

		if(userServerItemCurrent == NULL){
			break;
		}
	}

	//insert
	if(userServerItemS == NULL){
		userServerItem->next = NULL;
		userServerItem->prev = NULL;

		userServerItemS = userServerItem;
	} else {
		userServerItem->next = userServerItemS;
		userServerItem->prev = NULL;
		
		userServerItemS->prev = userServerItem;
		
		userServerItemS = userServerItem;
	}

	//character creater
	Character character;
	character.bomb = 1;
	character.position = (Position) {100,100};
	character.type = CharacterTypeUser;
	character.velocity = velocity;

	//character insert
	CharacterItem* characterItem = (CharacterItem*) malloc(sizeof(CharacterItem));
	characterItem->character = character;
	if(worldServer->characterItemS == NULL){
		characterItem->next = NULL;
		characterItem->prev = NULL;

		worldServer->characterItemS = characterItem;
	}else{
		characterItem->next = worldServer->characterItemS;
		characterItem->prev = NULL;

		worldServer->characterItemS->prev = characterItem;

		worldServer->characterItemS = characterItem;
	}

	//character associate
	userServer->character = &(characterItem->character);

	//reply
	strcpy(userServerUnsafe->auth, userServer->auth);
}
