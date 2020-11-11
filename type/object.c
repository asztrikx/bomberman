#include "../debugmalloc.h"
#include "object.h"
#include <stdlib.h>

//ObjectNew creates a new Object
Object* ObjectNew(){
	Object* object = (Object*) malloc(sizeof(Object));
	object->owner = NULL;

	return object;
}

//ObjectDelete frees Object
void ObjectDelete(Object* object){
	//free(object->owner) not handled by this
	free(object);
}
