#include "debugmalloc.h"
#include "geometry.h"
#include <math.h>
#include <stdint.h>
#include "config.h"
#include "type/geometry.h"
#include "type/object.h"
#include "type/character.h"

//CollisionPoint tells whether there's a collision between objects at positions
bool CollisionPoint(Position position1, Position position2){
	if(abs(position1.x - position2.x) >= squaresize){
		return false;
	}
	if(abs(position1.y - position2.y) >= squaresize){
		return false;
	}
	return true;
}

//CollisionPointAllObjectGet tells whether there's a collision between this and any Object
//if collisionDecideObjectFunction is NULL then it's treated as always true
List* CollisionPointAllObjectGet(List* objectS, Position position, void* this, CollisionDecideObjectFunction collisionDecideObjectFunction){
	List* listCollision = ListNew();

	for(ListItem* item = objectS->head; item != NULL; item = item->next){
		if(item->data == this){
			continue;
		}

		if(!CollisionPoint(position, ((Object*)item->data)->position)){
			continue;
		}

		if(
			collisionDecideObjectFunction != NULL &&
			!collisionDecideObjectFunction(this, item->data)
		){
			continue;
		}

		ListItem* listItem = (ListItem*) malloc(sizeof(ListItem));
		listItem->data = item->data;

		ListInsertItem(&listCollision, listItem);
	}

	return listCollision;
}

//CollisionPointAllCharacterGet tells whether there's a collision between this and any Character
//if collisionDecideCharacterFunction is NULL then it's treated as always true
List* CollisionPointAllCharacterGet(List* characterS, Position position, void* this, CollisionDecideCharacterFunction collisionDecideCharacterFunction){
	List* listCollision = ListNew();

	for(ListItem* item = characterS->head; item != NULL; item = item->next){
		if(item->data == this){
			continue;
		}

		if(!CollisionPoint(position, ((Character*)item->data)->position)){
			continue;
		}

		if(
			collisionDecideCharacterFunction != NULL &&
			!collisionDecideCharacterFunction(this, item->data)
		){
			continue;
		}

		ListItem* listItem = (ListItem*) malloc(sizeof(ListItem));
		listItem->data = item->data;

		ListInsertItem(&listCollision, listItem);
	}

	return listCollision;
}

//CollisionLinePositionGet tells whether there is collision with obstacle in discrete line (from, to)
//from must not be equal to to
//we can be NULL
Position CollisionLinePositionGet(
	WorldServer* worldServer,
	Position from,
	Position to,
	void* we,
	CollisionDecideObjectFunction collisionDecideObjectFunction,
	CollisionDecideCharacterFunction collisionDecideCharacterFunction
){
	//position difference in abs is always same for y and x coordinate if none of them is zero
	int step = abs(to.y - from.y);
	if (step == 0){
		step = abs(to.x - from.x);
	}
	Position current = from;

	//stays in place
	if(step == 0){
		return from;
	}

	//walk by discretely with unit vector as there are scenarios where
	//the collision would misbehave if we would only check the arrival position
	//eg: too fast speed would make it able to cross walls
	//eg: squaresize pixel wide diagonal is crossable this way
	Position unit = (Position){
		.y = 0,
		.x = 0,
	};
	if(to.y - from.y != 0){
		unit.y = (to.y - from.y) / abs(to.y - from.y);
	}
	if(to.x - from.x != 0){
		unit.x = (to.x - from.x) / abs(to.x - from.x);
	}
	for(int i=0; i<step; i++){
		//step y
		current.y += unit.y;

		List* collisionPointAllObject = CollisionPointAllObjectGet(worldServer->objectList, current, we, collisionDecideObjectFunction);
		List* collisionPointAllCharacter = CollisionPointAllCharacterGet(worldServer->characterList, current, we, collisionDecideCharacterFunction);
		if(
			collisionPointAllObject->length != 0 ||
			collisionPointAllCharacter->length != 0
		){
			current.y -= unit.y;
		}
		ListDelete(collisionPointAllObject, NULL);
		ListDelete(collisionPointAllCharacter, NULL);

		//step x
		current.x += unit.x;

		collisionPointAllObject = CollisionPointAllObjectGet(worldServer->objectList, current, we, collisionDecideObjectFunction);
		collisionPointAllCharacter = CollisionPointAllCharacterGet(worldServer->characterList, current, we, collisionDecideCharacterFunction);
		if(
			collisionPointAllObject->length != 0 ||
			collisionPointAllCharacter->length != 0
		){
			current.x -= unit.x;
		}
		ListDelete(collisionPointAllObject, NULL);
		ListDelete(collisionPointAllCharacter, NULL);
	}

	return current;
}

bool** collisionFreeCountObjectGetMemory;
//CollisionFreeCountObjectGetRecursion is a helper function of CollisionFreeCountObjectGet
int CollisionFreeCountObjectGetRecursion(WorldServer* worldServer, Position positionCompress){
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
	List* collisionListObject = CollisionPointAllObjectGet(worldServer->objectList, position, NULL, NULL);
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
		collisionFreeCountObject += CollisionFreeCountObjectGetRecursion(worldServer, positionCompressNew);
	}

	return collisionFreeCountObject;
}

//collisionFreeCountObject returns how many square sized object-free area is reachable from (position - position % squaresize)
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
	int count = CollisionFreeCountObjectGetRecursion(worldServer, positionCompress);

	//memory free
	free(collisionFreeCountObjectGetMemory[0]);
	free(collisionFreeCountObjectGetMemory);

	return count;
}

//SpawnGet return a position where there's at least 3 free space reachable without action so player does not die instantly
Position SpawnGet(WorldServer* worldServer, int collisionFreeCountObjectMin){
	//random max check
	if(
		RAND_MAX != INT32_MAX && (
			RAND_MAX + 1 < worldServer->height ||
			RAND_MAX + 1 < worldServer->width
		)
	){
		SDL_Log("WorldGenerate: map is too big");
		exit(1);
	}

	//position find
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
		List* collisionListCharacter = CollisionPointAllCharacterGet(worldServer->characterList, position, NULL, NULL);
		collisionCountCharacter = collisionListCharacter->length;
		ListDelete(collisionListCharacter, NULL);

		//position valid
		collisionFreeCountObject = CollisionFreeCountObjectGet(worldServer, position);
	} while (
		collisionCountCharacter != 0 ||
		collisionFreeCountObject < collisionFreeCountObjectMin
	);

	return position;
}
