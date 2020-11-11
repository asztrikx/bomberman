#include "../../debugmalloc.h"
#include "user.h"
#include <stdlib.h>

//UserServerNew creates a new UserServer
UserServer* UserServerNew(){
	UserServer* userServer = (UserServer*) malloc(sizeof(UserServer));
	userServer->auth = (char*) malloc((26 + 1) * sizeof(char));
	userServer->character = NULL;
	userServer->keyS = NULL;
	userServer->keySLength = 0;
	userServer->name = (char*) malloc((15 + 1) * sizeof(char));

	return userServer;
}

//UserServerDelete frees UserServer
void UserServerDelete(UserServer* userServer){
	free(userServer->auth);
	//free(userServer->character); //it is not free'd by this
	free(userServer->keyS);
	free(userServer->name);
	free(userServer);
}
