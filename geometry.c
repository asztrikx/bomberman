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
	if(height % 2 != 1 || width % 2 != 1){
		puts("World size is malformed");
		exit(1);
	}

	WorldServer* worldServer = (WorldServer*) malloc(sizeof(WorldServer));
	worldServer->objectItemS = NULL;
	worldServer->characterItemS = NULL;
	worldServer->exit = NULL;
	
	for(int i=0; i<height; i++){
		for(int j=0; j<width; j++){
			if(
				i == 0 || j == 0 ||
				i == height - 1 || j == width - 1 ||
				(i % 2 == 0 && j % 2 == 0)
			){
				objectItemSInsert(&(worldServer->objectItemS), &(Object){
					.created = -1,
					.position = (Position){
						.y = i * squaresize,
						.x = j * squaresize,
					},
					.type = ObjectTypeWall,
					.velocity = (Position){0,0},
				});
			}
		}
	}

	return worldServer;
}
