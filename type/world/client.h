#ifndef TYPE_CLIENT_WORLD_H_INCLUDED
#define TYPE_CLIENT_WORLD_H_INCLUDED

#include "../object.h"
#include "../character.h"
#include "../geometry.h"
#include "../gamestate.h"

typedef struct{
	Object* objectS;
	int objectSLength;
	Character* characterS;
	int characterSLength;
	Object* exit; //may not exists (for client)
	Gamestate gamestate;
} WorldClient;

WorldClient* WorldClientNew(void);
void WorldClientDelete(WorldClient* worldClient);

#endif
