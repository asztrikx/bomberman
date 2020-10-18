#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

#include "geometry.h"

typedef enum{
	ObjectTypeBomb,
	ObjectTypeObstacle,
	ObjectTypeWall,
} ObjectType;

typedef struct{
	Position position;
	ObjectType type;
	long long created; //destroy event => maybe destroyOn
} Object;

//bomb radius

#endif
