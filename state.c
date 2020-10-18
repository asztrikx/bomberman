#include "world.h"
#include "geometry.h"
#include <stdlib.h>

World* worldGenerate(int height, int width){
	World* world = (World*) malloc(sizeof(World));
	world->objectItemS = NULL;
	
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