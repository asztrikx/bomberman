#include "../../debugmalloc.h"
#include "client.h"
#include <stdlib.h>
#include <stdbool.h>

//UserClientNew creates a new UserClient
UserClient* UserClientNew(){
	UserClient* userClient = (UserClient*) malloc(sizeof(UserClient));
	userClient->auth = (char*) malloc((26 + 1) * sizeof(char));
	for(int i=0; i < KeyLength; i++){
		userClient->keyS[i] = false;
	}
	userClient->name = (char*) malloc((15 + 1) * sizeof(char));

	return userClient;
}

//UserClientDelete frees UserClient
void UserClientDelete(UserClient* userClient){
	free(userClient->auth);
	free(userClient->name);
	free(userClient);
}
