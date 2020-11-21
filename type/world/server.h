#ifndef TYPE_SERVER_WORLD_H_INCLUDED
#define TYPE_SERVER_WORLD_H_INCLUDED

#include "../list.h"
#include "../geometry.h"
#include "../object.h"

typedef struct{
	List* objectList;
	List* characterList;
	Object* exit; //not handled by this
	int height, width;
} WorldServer;

WorldServer* WorldServerNew();
void WorldServerDelete(WorldServer* worldServer);

#endif
