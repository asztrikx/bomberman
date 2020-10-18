#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#include "geometry.h"
#include "object.h"
#include "character.h"

typedef struct ObjectItem{
	Object* object;
	struct ObjectItem* next;
	struct ObjectItem* prev;
} ObjectItem;

typedef struct{
	ObjectItem* objectItemS; //use 2d array
	Character* characterS;
	Position exit;
} World;

#endif
