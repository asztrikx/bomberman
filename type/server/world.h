#ifndef TYPE_SERVER_WORLD_H_INCLUDED
#define TYPE_SERVER_WORLD_H_INCLUDED

#include "../list.h"
#include "../geometry.h"

typedef struct{
	List* objectList;
	List* characterList;
	Position* exit;
	int height, width;
} WorldServer;

WorldServer* WorldServerNew();
void WorldServerDelete(WorldServer* worldServer);

#endif
