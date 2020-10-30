#include "debugmalloc.h"
#include "geometry.h"
#include <math.h>
#include "state.h"

int squaresize = 50;
Position velocity = {10, 10};
int windowHeight = 480;
int windowWidth = 640;

bool collisionPoint(Position position1, Position position2){
	if(abs(position1.x - position2.x) >= squaresize){
		return false;
	}
	if(abs(position1.y - position2.y) >= squaresize){
		return false;
	}
	return true;
}

bool collisionLine(Position from, Position to, Position obstacle){
	//velocity is always has same abs value or one of them is zero so
	//to - from will have the same property
	int step = abs(to.y - from.y);
	if (step == 0){
		step = abs(to.x - from.x);
	}

	//stays in place
	if(step == 0){
		return collisionPoint(from, obstacle);
	}

	//step through each discrete value as there are scenarios where
	//the collision would misbehave if we would only check the arrival position
	//eg: too fast speed would make it able to cross walls
	//eg: squaresize pixel wide diagonal is crossable this way
	Position current = from;
	for(int i=0; i<step; i++){
		if(to.y - from.y != 0){
			current.y += (to.y - from.y) / abs(to.y - from.y);
		}
		if(to.x - from.x != 0){
			current.x += (to.x - from.x) / abs(to.x - from.x);
		}

		if(collisionPoint(current, obstacle)){
			return true;
		}
	}

	return false;
}

//collisionObjectS
//free must be called
ObjectItem* collisionObjectS(ObjectItem* objectItemS, Position from, Position to){
	ObjectItem* objectItemCollisionS = NULL;

	ObjectItem* objectItemCurrent = objectItemS;
	while(objectItemCurrent != NULL){
		if(collisionLine(from, to, objectItemCurrent->object->position)){
			ObjectItem* objectItem = (ObjectItem*) malloc(sizeof(ObjectItem));
			objectItem->object = objectItemCurrent->object;

			objectItemSInsertItem(&objectItemCollisionS, objectItem);
		}

		objectItemCurrent = objectItemCurrent->next;
	}
	
	return objectItemCollisionS;
}

//collisionCharacterS
//free must be called
CharacterItem* collisionCharacterS(CharacterItem* characterItemS, Position from, Position to){
	CharacterItem* characterItemCollisionS = NULL;

	CharacterItem* characterItemCurrent = characterItemS;
	while(characterItemCurrent != NULL){
		if(collisionLine(from, to, characterItemCurrent->character->position)){
			CharacterItem* characterItem = (CharacterItem*) malloc(sizeof(CharacterItem));
			characterItem->character = characterItemCurrent->character;

			characterItemSInsertItem(&characterItemCollisionS, characterItem);
		}

		characterItemCurrent = characterItemCurrent->next;
	}
	
	return characterItemCollisionS;
}

//worldGenerate generates default map
//free should be called
WorldServer* worldGenerate(int height, int width){
	if(height % 2 != 1 || width % 2 != 1){
		SDL_Log("World size is malformed");
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
				Object object = (Object){
					.created = -1,
					.position = (Position){
						.y = i * squaresize,
						.x = j * squaresize,
					},
					.type = ObjectTypeWall,
					.velocity = (Position){0,0},
					.bombOut = true,
					.owner = NULL,
				};
				objectItemSInsert(&(worldServer->objectItemS), &object);
			}
		}
	}

	return worldServer;
}
