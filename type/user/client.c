#include "../../debugmalloc.h"
#include "user.h"
#include <stdlib.h>
#include "../../state.h"

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
	ListDelete(userClient->keyList, intfree);
	free(userClient->name);
	free(userClient);
}
