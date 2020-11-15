#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED

#include <stdbool.h>
#include "type/geometry.h"
#include "type/list.h"
#include "type/world/server.h"
#include "type/object.h"
#include "type/character.h"

//CollisionDecideObjectFunction first paramtere is this
typedef bool (*CollisionDecideObjectFunction)(void*, Object*);
//CollisionDecideCharacterFunction first paramtere is this
typedef bool (*CollisionDecideCharacterFunction)(void*, Character*);

bool CollisionPoint(Position position1, Position position2);
List* CollisionPointAllObjectGet(List* objectS, Position position, void* this, CollisionDecideObjectFunction collisionDecideObjectFunction);
List* CollisionPointAllCharacterGet(List* characterS, Position position, void* this, CollisionDecideCharacterFunction collisionDecideCharacterFunction);
Position CollisionLinePositionGet(
	WorldServer* worldServer,
	Position from,
	Position to,
	void* we,
	CollisionDecideObjectFunction collisionDecideObjectFunction,
	CollisionDecideCharacterFunction collisionDecideCharacterFunction
);
Position SpawnGet(WorldServer* worldServer, int collisionFreeCountObjectMin);
int CollisionFreeCountObjectGet(WorldServer* worldServer, Position position);

#endif
