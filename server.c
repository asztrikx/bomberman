#include "server.h"
#include "state.h"
#include "network.h"

UserItem* userItemS;
World* world;

void ServerWorldGenerate(int height, int width){
	world = (World*) malloc(sizeof(World));
	world->objectItemS = NULL;
	world->characterItemS = NULL;
	world->exit = NULL;
	
	for(int i=0; i<height; i++){
		int y = i * squaresize;
		int x = 0;

		//object
		Object* object = (Object*) malloc(sizeof(Object));
		object->position = (Position){
			.y = y,
			.x = x,
		};
		object->type = ObjectTypeWall;

		//objectItem
		ObjectItem* objectItem = (ObjectItem*) malloc(sizeof(ObjectItem));
		objectItem->object = object;
		
		//objectItemS first element
		if(world->objectItemS == NULL){
			objectItem->next = NULL;
			objectItem->prev = NULL;

			world->objectItemS = objectItem;

			continue;
		}

		//objectItemS insert front
		objectItem->next = world->objectItemS;
		objectItem->prev = NULL;

		world->objectItemS->prev = objectItem;

		world->objectItemS = objectItem;
	}

	return world;
}

void ServerStart(void){
	ServerWorldGenerate(10, 10);
}

//ServerReceive gets updates from users
void ServerReceive(User* user){
	//auth
	UserItem* userItem = userItemS;
	while(userItem != NULL){
		//not timing attack safe compare
		if(strcmp(user->name, userItem->user.auth) == 0){
			break;
		}

		userItem = userItem->next;
	}

	if(userItem == NULL){
		return;
	}

	//change name
	if(strcmp(userItem->user.name, user->name) != 0){
		strcpy(userItem->user.name, user->name); //length check
		userItem->user.nameLength = user->nameLength; //validate
	}

	//key apply
	KeyItem* keyItem = user->keyItemS;
	while(keyItem != NULL){
		if(keyItem->key == SDLK_w){
			userItem->user.character->velocity.y -= 10;
		} else if(keyItem->key == ...)
	}
}

//ClientSend sends updates to server
Uint32 ServerSend(Uint32 interval, void *param){
	networkSendClient(world); //send only seeable parts

	return interval;
}

void ServerStop(void){
}

User* ServerConnect(User* user){
	//check if game is running

	//add character with id

	//malformed struct
	if(user->auth != NULL){
		printf("ServerJoin: id not null");
		return NULL;
	}
	
	if(user->ablityS != NULL){
		printf("ServerJoin: abilityS not null");
		return NULL;
	}

	if(user->keyItemS != NULL){
		printf("ServerJoin: keyItems not null");
		return NULL;
	}

	if(user->name == NULL){
		printf("ServerJoin: name is null");
		return NULL;
	}

	//name length check

	//id generate
	char* id = (char*) malloc(26 * sizeof(char)); //30years to crack
	while (true){
		for(int i=0; i<26; i++){
			id[i] = rand() % ('Z' - 'A' + 1) + 'A';
		}
		
		UserItem* userItemCurrent = userItemS;
		while(userItemCurrent != NULL){
			if(strcmp(userItemCurrent->user.auth, id) == 0){
				break;
			}		
			userItemCurrent = userItemCurrent->next;
		}

		if(userItemCurrent == NULL){
			break;
		}
	}
	user->auth = id;

	//insert
	UserItem* userItem = (UserItem*) malloc(sizeof(UserItem));
	userItem->user = *user; //copy as it will be freed by caller
	if(userItemS == NULL){
		userItemS = userItem;
		userItemS->next = NULL;
		userItemS->prev = NULL;
	} else {
		userItem->next = userItemS;
		userItem->prev = NULL;
		
		userItemS->prev = userItem;
		
		userItemS = userItem;
	}

	//character insert
	CharacterItem* characterItem = (CharacterItem*) malloc(sizeof(CharacterItem));
	characterItem->character->type = CharacterTypeUser;
	if(world->characterItemS == NULL){
		characterItem->next = NULL;
		characterItem->prev = NULL;

		world->characterItemS = characterItem;
	}else{
		characterItem->next = world->characterItemS;
		characterItem->prev = NULL;

		world->characterItemS->prev = characterItem;

		world->characterItemS = characterItem;
	}

	return user;
}

	/*int deleteme = 10;
	//key
	switch(sdl_event.key.keysym.sym){
		case SDLK_w:
			characterS[0].position.y -= deleteme;
			break;
		case SDLK_a:
			characterS[0].position.x -= deleteme;
			break;
		case SDLK_s:
			characterS[0].position.y += deleteme;
			break;
		case SDLK_d:
			characterS[0].position.x += deleteme;
			break;
	}*/

	/*

	objectS = (Object*) malloc(10 * sizeof(Object));
	objectS[0] = (Object){
		.type = ObjectTypeWall,
		.position = (Position){
			.y = 0,
			.x = 0,
		},
	};
	objectSLength = 1;

	characterS = (Character*) malloc(10 * sizeof(Character));
	characterS[0] = (Character){
		.position = (Position){
			.y = 100,
			.x = 100,
		},
		.type = CharacterTypeUser,
		.ablityS = NULL,
	};
	characterSLength = 1;
	*/
