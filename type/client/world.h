#ifndef TYPE_CLIENT_WORLD_H_INCLUDED
#define TYPE_CLIENT_WORLD_H_INCLUDED

#include "../object.h"
#include "../character.h"
#include "../geometry.h"

typedef struct{
	Object* objectS;
	int objectSLength;
	Character* characterS;
	int characterSLength;
	Position* exit; //may not exists (for client)
} WorldClient;

WorldClient* WorldClientNew();
void WorldClientDelete(WorldClient* worldClient);

#endif
