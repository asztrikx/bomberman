#include "server.h"
#include "state.h"
#include "network.h"

static UserItem* userItemS;
static World* world;
static long long tickCount = 0;
static unsigned int tickRate = 60u;

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
}

Uint32 ServerTick(Uint32 interval, void *param){
	//[R]bomb explosion
	//[R]player death
	//[R]crate delete

	//[R] send to clients
	networkSendClient(world);

	return interval;
}

void ServerStart(void){
	ServerWorldGenerate(10, 10);

	SDL_AddTimer(1000u/tickRate, ServerTick, NULL);
	//start tick
}

//ServerReceive gets updates from users
void ServerReceive(User* user){
	//auth
	UserItem* userItem = userItemS;
	while(userItem != NULL){
		//[R] not timing attack safe compare
		if(strcmp(user->auth, userItem->user.auth) == 0){
			break;
		}

		userItem = userItem->next;
	}

	if(userItem == NULL){
		return;
	}
	User* userAuth = &(userItem->user);

	//change name
	if(strcmp(userAuth->name, user->name) != 0){
		strcpy(userAuth->name, user->name); //[R] length check
	}

	//key apply
	KeyItem* keyItem = user->keyItemS;
	while(keyItem != NULL){
		printf("Received: %c\n", keyItem->key);
		if(keyItem->key == SDLK_SPACE){
			//position
			Position position = userAuth->character->position;
			if (userAuth->character->position.y % squaresize > squaresize / 2){
				position.y += 1;
			}
			if (userAuth->character->position.x % squaresize > squaresize / 2){
				position.x += 1;
			}

			//bomb create
			Object* object = (Object*) malloc(sizeof(Object));
			object->type = ObjectTypeBomb;
			object->created = tickCount;
			object->position = position;
			object->velocity = (Position){0, 0};

			//bomb insert
			//position is empty
			ObjectItem* objectItem = (ObjectItem*) malloc(sizeof(Object));
			if(world->objectItemS == NULL){
				objectItem->prev = NULL;
				objectItem->next = NULL;
				world->objectItemS = objectItem;
			} else {
				objectItem->prev = NULL;
				objectItem->next = world->objectItemS;
				
				world->objectItemS->prev = objectItem;

				world->objectItemS = objectItem;
			}
		} else if(keyItem->key == SDLK_w){
			//[R]collision

			userAuth->character->position.y -= userAuth->character->velocity.y;
		} else if(keyItem->key == SDLK_a){
			//[R]collision

			userAuth->character->position.x -= userAuth->character->velocity.x;
		} else if(keyItem->key == SDLK_s){
			//[R]collision

			userAuth->character->position.y += userAuth->character->velocity.y;
		} else if(keyItem->key == SDLK_d){
			//[R]collision

			userAuth->character->position.x += userAuth->character->velocity.x;
		}
		keyItem = keyItem->next;
	}
}

void ServerStop(void){
}

User* ServerConnect(User* user){
	//[R]check if game is running

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

	//[R]name length check

	//id generate
	char* id = (char*) malloc((26 + 1) * sizeof(char)); //30years to crack
	while (true){
		for(int i=0; i<26; i++){
			id[i] = rand() % ('Z' - 'A' + 1) + 'A';
		}
		id[26] = '\0';
		
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

	//character creater
	Character character;
	character.bomb = 1;
	character.position = (Position) {100,100};
	character.type = CharacterTypeUser;
	character.velocity = velocity;

	//character insert
	CharacterItem* characterItem = (CharacterItem*) malloc(sizeof(CharacterItem));
	characterItem->character = character;
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

	//character associate
	userItem->user.character = &(characterItem->character);

	return user;
}
