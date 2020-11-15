#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED

#include "type/geometry.h"
#include "type/list.h"
#include "type/world/server.h"

bool CollisionPointGet(Position position1, Position position2);
bool CollisionLineGet(Position from, Position to, Position obstacle);
List* CollisionObjectSGet(List* list, Position from, Position to);
List* CollisionCharacterSGet(List* list, Position from, Position to);
Position SpawnGet(WorldServer* worldServer, int collisionFreeCountObjectMin);
int CollisionFreeCountObjectGet(WorldServer* worldServer, Position position);

#endif
