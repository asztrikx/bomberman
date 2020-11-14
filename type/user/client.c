#include "../../debugmalloc.h"
#include "client.h"
#include <stdlib.h>
#include "../../state.h"
#include "../int.h"

//UserClientNew creates a new UserClient
UserClient* UserClientNew(){
	UserClient* userClient = (UserClient*) malloc(sizeof(UserClient));
	//userClient->ablityList = ListNew();
	userClient->auth = (char*) malloc((26 + 1) * sizeof(char));
	userClient->keyList = ListNew();
	userClient->name = (char*) malloc((15 + 1) * sizeof(char));

	return userClient;
}

//UserClientDelete frees UserClient
void UserClientDelete(UserClient* userClient){
	//ListDelete(userClient->ablityList, );
	free(userClient->auth);
	ListDelete(userClient->keyList, IntDelete);
	free(userClient->name);
	free(userClient);
}
