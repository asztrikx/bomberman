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

}

//ClientSend sends updates to server
Uint32 ServerSend(Uint32 interval, void *param){
	networkSendClient(world);

	return interval;
}

void ServerStop(void){
	/*CharacterItem* current = ServerWorld.characterItemS;
	while(current != NULL){
		CharacterItem* next = current->next;
		current = NULL;/** & free
		ServerWorld.characterItemS = NULL;
	}*/
	//free
	//ServerWorld.objectItemS = NULL;
}

void ServerJoin(User user){
	//check if game is running

	//add to userItems
	//generate id
	//add character with id
	//send id
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
