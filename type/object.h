#ifndef TYPE_OBJECT_H_INCLUDED
#define TYPE_OBJECT_H_INCLUDED

#include "geometry.h"
#include "character.h"
#include "animation.h"

typedef enum{
	ObjectTypeBomb,
	ObjectTypeBombFire,
	ObjectTypeWall,
	ObjectTypeBox,
	ObjectTypeExit,
} ObjectType;

typedef struct{
	Position position;
	ObjectType type;
	long long created;
	long long destroy; //-1 means never gets destroyed by server
	Position velocity;
	Character* owner; //NULL if server or disconnected player
	bool bombOut; //only for ObjectTypeBomb signaling whether player has moved out from bomb
	Animation animation;
} Object;

Object* ObjectNew();
void ObjectDelete(Object* object);

#endif
