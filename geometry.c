#include "debugmalloc.h"
#include "geometry.h"
#include <math.h>
#include <stdint.h>
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

//collisionObjectS return all collisions in line (from, to)
//returned list is a reference list, which must be freed without deleting data
List* collisionObjectS(List* list, Position from, Position to){
	List* listCollision = ListNew();

	for(ListItem* item = list->head; item != NULL; item = item->next){
		if(collisionLine(from, to, ((Object*)item->data)->position)){
			ListItem* listItem = (ListItem*) malloc(sizeof(ListItem));
			listItem->data = item->data;

			ListInsertItem(&listCollision, listItem);
		}
	}
	
	return listCollision;
}

//collisionCharacterS return all collisions in line (from, to)
//returned list is a reference list, which must be freed without deleting data
List* collisionCharacterS(List* list, Position from, Position to){
	List* listCollision = ListNew();

	for(ListItem* item = list->head; item != NULL; item = item->next){
		if(collisionLine(from, to, ((Character*)item->data)->position)){
			ListItem* listItem = (ListItem*) malloc(sizeof(ListItem));
			listItem->data = item->data;

			ListInsertItem(&listCollision, listItem);
		}
	}
	
	return listCollision;
}

//worldGenerate generates default map
//free should be called
WorldServer* worldGenerate(int height, int width){
	if(height % 2 != 1 || width % 2 != 1){
		SDL_Log("worldGenerate: World size is malformed");
		exit(1);
	}

	WorldServer* worldServer = WorldServerNew();
	worldServer->height = height;
	worldServer->width = width;
	
	for(int i=0; i<height; i++){
		for(int j=0; j<width; j++){
			if(
				i == 0 || j == 0 ||
				i == height - 1 || j == width - 1 ||
				(i % 2 == 0 && j % 2 == 0)
			){
				//[R] check collision
				Object* object = ObjectNew();
				object->created = -1;
				object->destroy = -1;
				object->position = (Position){
						.y = i * squaresize,
						.x = j * squaresize,
					};
				object->type = ObjectTypeWall;
				object->velocity = (Position){0,0};
				object->bombOut = true;
				object->owner = NULL;
				ListInsert(&(worldServer->objectList), object);
			}
		}
	}
	
	//box generate
	if(
		RAND_MAX != INT32_MAX && (
			RAND_MAX + 1 < height ||
			RAND_MAX + 1 < width
		)
	){
		SDL_Log("worldGenerate: map is too big");
		exit(1);
	}
	for(int i=0; i<(width + 1) * (height + 1) * 0.2; i++){
		int y = 0, x = 0;
		while(
			y == 0 || x == 0 ||
			y == height - 1 || x == width - 1 ||
			(y % 2 == 0 && x % 2 == 0)
		){
			y = rand() % height;
			x = rand() % width;
		}

		Object* object = ObjectNew();
		object->created = -1;
		object->destroy = -1;
		object->position = (Position){
				.y = y * squaresize,
				.x = x * squaresize,
			};
		object->type = ObjectTypeBox;
		object->velocity = (Position){0,0};
		object->bombOut = true;
		object->owner = NULL;
		ListInsert(&(worldServer->objectList), object);
	}

	return worldServer;
}
