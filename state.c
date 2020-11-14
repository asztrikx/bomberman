#include "debugmalloc.h"
#include "state.h"

void* Copy(void* data, size_t size){
	unsigned char* copy = (unsigned char*) malloc(size * sizeof(char));
	memcpy(copy, data, size);
	return copy;
}

Ability AbilitySpeedExtra = {
	.speedExtra = 10,
};
