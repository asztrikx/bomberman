#include "../../debugmalloc.h"
#include "world.h"
#include <stdlib.h>
#include "../character.h"
#include "../object.h"
#include "../geometry.h"

//WorldClientNew creates a new WorldClient
WorldClient* WorldClientNew(){
	WorldClient* worldClient = (WorldClient*) malloc(sizeof(WorldClient));
	worldClient->characterS = NULL;
	worldClient->characterSLength = 0;
	worldClient->exit = NULL;
	worldClient->objectS = NULL;
	worldClient->objectSLength = 0;

	return worldClient;
}

//WorldClientDelete frees worldClient
void WorldClientDelete(WorldClient* worldClient){
	free(worldClient->characterS);
	free(worldClient->exit);
	free(worldClient->objectS);
	free(worldClient);
}
