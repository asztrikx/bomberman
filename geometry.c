#include "debugmalloc.h"
#include "geometry.h"
#include "state.h"

int squaresize = 50;
Position velocity = {10, 10};
int windowHeight = 480;
int windowWidth = 640;

bool collisionPositionS(Position position1, Position position2){
	if(abs(position1.x - position2.x) >= squaresize){
		return false;
	}
	if(abs(position1.y - position2.y) >= squaresize){
		return false;
	}
	return true;
}

bool collisionObjectS(ObjectItem* objectItems, Position position){
	ObjectItem* objectItem = objectItems;
	while(objectItem != NULL){
		if(collisionPositionS(objectItem->object.position, position)){
			return true;
		}

		objectItem = objectItem->next;
	}
	
	return false;
}

bool collisionCharacterS(CharacterItem* characterItemS, Position position, Character* characterException){
	CharacterItem* characterItem = characterItemS;
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

//worldGenerate generates default map
//free should be called
WorldServer* worldGenerate(int height, int width){
	WorldServer* worldServer = (WorldServer*) malloc(sizeof(WorldServer));
	worldServer->objectItemS = NULL;
	worldServer->characterItemS = NULL;
	worldServer->exit = NULL;
	
	for(int i=0; i<height; i++){
		int y = i * squaresize;
		int x = 0;

		//objectItem
		ObjectItem* objectItem = (ObjectItem*) malloc(sizeof(ObjectItem));
		objectItem->object = (Object){
			.created = -1,
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

	return worldServer;
}
