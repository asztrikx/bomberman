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

typedef struct CharacterItem{
	Character* character;
	struct CharacterItem* next;
	struct CharacterItem* prev;
} CharacterItem;

typedef struct{
	ObjectItem* objectItemS; //use 2d array
	Character* characterItemS;
	Position exit;
} World;

#endif
