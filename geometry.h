#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED

#include "type/geometry.h"
#include "type/list.h"
#include "type/world/server.h"

bool collisionPoint(Position position1, Position position2);
bool collisionLine(Position from, Position to, Position obstacle);
List* collisionObjectS(List* list, Position from, Position to);
List* collisionCharacterS(List* list, Position from, Position to);
Position spawnGet(WorldServer* worldServer);
int collisionFreeCountObjectGet(WorldServer* worldServer, Position position);

#endif
