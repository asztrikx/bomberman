#include "../debugmalloc.h"
#include "object.h"
#include "animation.h"
#include <stdlib.h>

//ObjectNew creates a new Object
Object* ObjectNew(){
	Object* object = (Object*) malloc(sizeof(Object));
	object->created = -1;
	object->destroy = -1;
	object->position = (Position){
			.y = 0,
			.x = 0,
		};
	object->velocity = (Position){
		.y = 0,
		.x = 0,
	};
	object->bombOut = false;
	object->owner = NULL;
	object->animation = (Animation){
		.state = 0,
		.stateDelayTick = 0,
		.stateDelayTickEnd = 0,
	};

	return object;
}

//ObjectDelete frees Object
void ObjectDelete(Object* object){
	//free(object->owner) not handled by this
	free(object);
}
