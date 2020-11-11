#ifndef TYPE_OBJECT_H_INCLUDED
#define TYPE_OBJECT_H_INCLUDED

#include "geometry.h"
#include "character.h"

typedef enum{
	ObjectTypeBomb,
	ObjectTypeBombFire,
	ObjectTypeObstacle,
	ObjectTypeWall,
	ObjectTypeBox,
} ObjectType;

typedef struct{
	Position position;
	ObjectType type;
	long long created;
	long long destroy; //-1 means never gets destroyed by server
	Position velocity;
	Character* owner; //NULL if server or disconnected player
	bool bombOut; //only for ObjectTypeBomb signaling whether player has move out from bomb
} Object;

Object* ObjectNew();
void ObjectDelete(Object* object);

#endif
