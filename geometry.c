#include "debugmalloc.h"
#include "geometry.h"
#include <math.h>
#include <stdint.h>
#include "config.h"
#include "type/geometry.h"
#include "type/object.h"

bool CollisionPointGet(Position position1, Position position2){
	if(abs(position1.x - position2.x) >= squaresize){
		return false;
	}
	if(abs(position1.y - position2.y) >= squaresize){
		return false;
	}
	return true;
}

//CollisionLineGet tells whether there is collision with obstacle in discrete line (from, to)
bool CollisionLineGet(Position from, Position to, Position obstacle){
	//velocity is always has same abs value or one of them is zero so
	//to - from will have the same property
	int step = abs(to.y - from.y);
	if (step == 0){
		step = abs(to.x - from.x);
	}

	//stays in place
	if(step == 0){
		return CollisionPointGet(from, obstacle);
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
		if(CollisionPointGet(current, obstacle)){
			return true;
		}

		if(to.x - from.x != 0){
			current.x += (to.x - from.x) / abs(to.x - from.x);
		}
		if(CollisionPointGet(current, obstacle)){
			return true;
		}
	}

	return false;
}

//CollisionObjectSGet return all collisions in line (from, to)
//returned list is a reference list, which must be freed without deleting data
List* CollisionObjectSGet(List* list, Position from, Position to){
	List* listCollision = ListNew();

	for(ListItem* item = list->head; item != NULL; item = item->next){
		if(CollisionLineGet(from, to, ((Object*)item->data)->position)){
			ListItem* listItem = (ListItem*) malloc(sizeof(ListItem));
			listItem->data = item->data;

			ListInsertItem(&listCollision, listItem);
		}
	}
	
	return listCollision;
}

//CollisionCharacterSGet return all collisions in line (from, to)
//returned list is a reference list, which must be freed without deleting data
List* CollisionCharacterSGet(List* list, Position from, Position to){
	List* listCollision = ListNew();

	for(ListItem* item = list->head; item != NULL; item = item->next){
		if(CollisionLineGet(from, to, ((Character*)item->data)->position)){
			ListItem* listItem = (ListItem*) malloc(sizeof(ListItem));
			listItem->data = item->data;

			ListInsertItem(&listCollision, listItem);
		}
	}
	
	return listCollision;
}

bool** collisionFreeCountObjectGetMemory = NULL;
int collisionFreeCountObjectGetRecursion(WorldServer* worldServer, Position positionCompress){
	Position position;
	position.y = positionCompress.y * squaresize;
	position.x = positionCompress.x * squaresize;

	//in calculation or already calculated
	if(collisionFreeCountObjectGetMemory[positionCompress.y][positionCompress.x]){
		return 0;
	}

	//because of map border there will be no overindexing
	//mark invalid positions also to save collision recalculation
	collisionFreeCountObjectGetMemory[positionCompress.y][positionCompress.x] = true;

	//position is valid
	List* collisionListObject = CollisionObjectSGet(worldServer->objectList, position, position);
	int collisionCount = collisionListObject->length;
	ListDelete(collisionListObject, NULL);

	if(collisionCount != 0){
		return 0;
	}

	//neighbour positions
	int collisionFreeCountObject = 1; //current position
	int directionY[] = {1, -1, 0, 0};
	int directionX[] = {0, 0, 1, -1};
	for(int i=0; i < 4; i++){
		Position positionCompressNew = (Position){
			.y = positionCompress.y + directionY[i],
			.x = positionCompress.x + directionX[i],
		};
		collisionFreeCountObject += collisionFreeCountObjectGetRecursion(worldServer, positionCompressNew);
	}

	return collisionFreeCountObject;
}

//collisionFreeCountObject return how many square sized object-free area is reachable from (position / squaresize)
int CollisionFreeCountObjectGet(WorldServer* worldServer, Position position){
	//memory alloc
	collisionFreeCountObjectGetMemory = (bool**) malloc(worldServer->height * sizeof(bool*));
	collisionFreeCountObjectGetMemory[0] = (bool*) calloc(worldServer->height * worldServer->width, sizeof(bool));
	for(int i=1; i<worldServer->height; i++){
		collisionFreeCountObjectGetMemory[i] = collisionFreeCountObjectGetMemory[0] + i * worldServer->width;
	}

	//recursion
	Position positionCompress;
	positionCompress.y = position.y / squaresize;
	positionCompress.x = position.x / squaresize;
	int count = collisionFreeCountObjectGetRecursion(worldServer, positionCompress);

	//memory free
	free(collisionFreeCountObjectGetMemory[0]);
	free(collisionFreeCountObjectGetMemory);

	return count;
}

//SpawnGet return a position where there's at least 3 free space reachable without action so player does not die instantly
Position SpawnGet(WorldServer* worldServer){
	Position positionCompressed;
	Position position;
	int collisionCountCharacter;
	int collisionFreeCountObject;
	do {
		//random position in world
		//this could be a bit optimized but it's more error prone
		positionCompressed.y = rand() % worldServer->height;
		positionCompressed.x = rand() % worldServer->width;

		//decompress
		position.y = positionCompressed.y * squaresize;
		position.x = positionCompressed.x * squaresize;

		//collision check
		List* collisionListCharacter = CollisionCharacterSGet(worldServer->objectList, position, position);
		collisionCountCharacter = collisionListCharacter->length;
		ListDelete(collisionListCharacter, NULL);

		//position valid
		collisionFreeCountObject = CollisionFreeCountObjectGet(worldServer, position);
	} while (
		collisionCountCharacter != 0 ||
		collisionFreeCountObject <= 2
	);

	return position;
}
