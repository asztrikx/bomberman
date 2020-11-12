#include "../debugmalloc.h"
#include "array.h"
#include <stdlib.h>
#include <math.h>

Array* ArrayNew(size_t size){
	Array* array = (Array*) malloc(sizeof(Array));
	array->length = 0;
	array->capacity = 1;
	array->data = (void**) malloc(array->capacity * size);

	return array;
}

void ArrayInsert(Array* array, void* value, size_t size){
	if(array->length == array->capacity){
		//dataNew
		void** dataNew = (void**) malloc(2 * array->capacity * size);
		memcpy(dataNew, array->data, array->capacity * size);

		//replace
		free(array->data);
		array->data = dataNew;

		//capacity
		array->capacity *= 2;
	}

	array->data[array->length] = value;
	array->length++;
}

void ArrayDelete(Array* array, void(*dataFree)(void*)){
	if(dataFree != NULL){
		for(int i=0; i < array->length; i++){
			dataFree(array->data[i]);
		}
	}
	
	free(array->data);
	free(array);
}
