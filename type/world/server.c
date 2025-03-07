#include "../../debugmalloc.h"
#include "server.h"
#include <stdlib.h>
#include "../character.h"
#include "../object.h"

//WorldServerNew creates a new WorldServer
WorldServer* WorldServerNew(void){
	WorldServer* worldServer = (WorldServer*) malloc(sizeof(WorldServer));
	worldServer->characterList = ListNew();
	worldServer->exit = NULL;
	worldServer->height = 0;
	worldServer->objectList = ListNew();
	worldServer->width = 0;

	return worldServer;
}

//WorldServerDelete frees WorldServer
void WorldServerDelete(WorldServer* worldServer){
	ListDelete(worldServer->characterList, CharacterDelete);
	//ObjectDelete(worldServer->exit); not handled by this
	ListDelete(worldServer->objectList, ObjectDelete);
	free(worldServer);
}
