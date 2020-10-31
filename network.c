#include "debugmalloc.h"
#include "network.h"
#include <SDL2/SDL.h>
#include "server.h"
#include "client.h"
#include "state.h"

bool serverStop = true;
void networkServerStop(void){
	serverStop = true;
}
void networkServerStart(void){
	serverStop = false;
}

bool clientStop = true;
void networkClientStop(void){
	clientStop = true;
}
void networkClientStart(void){
	clientStop = false;
}

//networkSendClient send worldServer to client as WorldClient
//worldServer is not locked with mutex
void networkSendClient(WorldServer* worldServer){
	WorldClient* worldClient = (WorldClient*) malloc(sizeof(WorldClient));

	//exit
	if(worldServer->exit == NULL){
		worldClient->exit = NULL;
	} else {
		worldClient->exit = (Position*) malloc(sizeof(Position));
		*(worldClient->exit) = *(worldServer->exit);
	}
	
	//objectS length
	int objectSLength = 0;
	ObjectItem* objectItemCurrent = worldServer->objectItemS;
	while(objectItemCurrent != NULL){
		++objectSLength;

		objectItemCurrent = objectItemCurrent->next;
	}
	worldClient->objectSLength = objectSLength;

	//objectS
	worldClient->objectS = (Object*) malloc(objectSLength * sizeof(Object));
	objectItemCurrent = worldServer->objectItemS;
	for(int i=0; i<objectSLength; i++){
		worldClient->objectS[i] = *(objectItemCurrent->object);

		objectItemCurrent = objectItemCurrent->next;
	}

	//characterS length
	int characterSLength = 0;
	CharacterItem* characterItemCurrent = worldServer->characterItemS;
	while(characterItemCurrent != NULL){
		++characterSLength;

		characterItemCurrent = characterItemCurrent->next;
	}
	worldClient->characterSLength = characterSLength;

	//characterS
	worldClient->characterS = (Character*) malloc(characterSLength * sizeof(Character));
	characterItemCurrent = worldServer->characterItemS;
	for(int i=0; i<characterSLength; i++){
		worldClient->characterS[i] = *(characterItemCurrent->character);

		characterItemCurrent = characterItemCurrent->next;
	}

	//send
	if(!clientStop){ //network abstraction
		ClientReceive(worldClient);
	}

	//free
	free(worldClient->exit);
	free(worldClient->objectS);
	free(worldClient->characterS);
	free(worldClient);
}

//networkSendServer send userClient to server as UserServer
//keyItemS length must be greater than zero
void networkSendServer(UserClient* userClient){
	//keyItemS length
	int keySLength = 0;
	KeyItem* keyItemCurrent = userClient->keyItemS;
	while(keyItemCurrent != NULL){
		++keySLength;

		keyItemCurrent = keyItemCurrent->next;
	}

	//keyItemS copy
	SDL_Keycode* keyS = (SDL_Keycode*) malloc(keySLength * sizeof(SDL_Keycode));
	keyItemCurrent = userClient->keyItemS;
	for(int i=0; i<keySLength; i++){
		keyS[i] = keyItemCurrent->key;

		keyItemCurrent = keyItemCurrent->next;
	}

	//create userServer
	UserServer userServerCopy = (UserServer){
		.auth = (char*) malloc((26 + 1) * sizeof(char)),
		.character = NULL,
		.keyS = keyS,
		.keySLength = keySLength,
		.name = (char*) malloc((15 + 1) * sizeof(char)),
	};
	UserServer* userServer = &userServerCopy;
	strcpy(userServer->auth, userClient->auth);
	strcpy(userServer->name, userClient->name);

	//send
	if(!serverStop){
		ServerReceive(userServer); //network abstraction
	}

	//free
	free(userServer->auth);
	free(userServer->character); //in best case it's free(NULL)
	free(userServer->keyS);
	free(userServer->name);
}

//networkConnectServer client request to server to create connection
void networkConnectServer(UserClient* userClient){
	//copy
	UserServer userServerCopy = (UserServer){
		.auth = NULL,
		.character = NULL,
		.keyS = NULL,
		.keySLength = 0,
		.name = (char*) malloc((15 + 1) * sizeof(char)),
	};
	UserServer* userServer = &userServerCopy;
	strcpy(userServer->name, userClient->name);

	//send
	if(!serverStop){
		ServerConnect(userServer); //network abstraction

		//receive
		//network abstraction
	}

	//[R] handle timeout

	//apply changes
	free(userClient->auth); //in best case it's free(NULL)
	userClient->auth = (char*) malloc((26 + 1) * sizeof(char));

	strncpy(userClient->auth, userServer->auth, 26);
	userClient->auth[26] = '\0';
	strncpy(userClient->name, userServer->name, 15); //name could be occupied
	userClient->name[15] = '\0';

	//free
	free(userServer->name);
	free(userServer->auth); //reply from server
}
