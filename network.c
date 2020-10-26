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
		worldClient->objectS[i] = objectItemCurrent->object;

		objectItemCurrent = objectItemCurrent->next;
	}

	//characterS
	int characterSLength = 0;
	CharacterItem* characterItemCurrent = worldServer->characterItemS;
	while(characterItemCurrent != NULL){
		++characterSLength;

		characterItemCurrent = characterItemCurrent->next;
	}
	worldClient->characterSLength = characterSLength;

	worldClient->characterS = (Character*) malloc(characterSLength * sizeof(Character));
	characterItemCurrent = worldServer->characterItemS;
	for(int i=0; i<characterSLength; i++){
		worldClient->characterS[i] = characterItemCurrent->character;

		characterItemCurrent = characterItemCurrent->next;
	}

	//send
	ClientReceive(worldClient); //network abstraction

	//free
	free(worldClient->exit);
	free(worldClient->objectS);
	free(worldClient->characterS);
	free(worldClient);
}

//networkSendServer send userClient to server as UserServer
void networkSendServer(UserClient* userClient){
	//copy keyItemS
	int keySLength = 0;
	KeyItem* keyItem = userClient->keyItemS;
	while(keyItem != NULL){
		++keySLength;

		keyItem = keyItem->next;
	}

	SDL_Keycode* keyS = (SDL_Keycode*) malloc(keySLength * sizeof(SDL_Keycode));//NULL if 0
	keyItem = userClient->keyItemS;
	for(int i=0; i<keySLength; i++){
		keyS[i] = keyItem->key;

		keyItem = keyItem->next;
	}

	//create userServer
	UserServer* userServer = (UserServer*) malloc(sizeof(UserServer));
	userServer->auth = (char*) malloc((26 + 1) * sizeof(char));
	userServer->character = NULL;
	userServer->keyS = keyS;
	userServer->keySLength = keySLength;
	userServer->name = (char*) malloc((15 + 1) * sizeof(char));

	strcpy(userServer->auth, userClient->auth);
	strcpy(userServer->name, userClient->name);

	//send
	if(!serverStop){
		ServerReceive(userServer); //network abstraction
	}

	//free
	free(userServer->auth);
	free(userServer->keyS);
	free(userServer->name);
	free(userServer);
}

//networkConnectServer client request to server to create connection
void networkConnectServer(UserClient* userClient){
	//copy
	UserServer* userServer = (UserServer*) malloc(sizeof(UserServer));
	userServer->auth = NULL;
	userServer->character = NULL;
	userServer->keyS = NULL;
	userServer->keySLength = 0;
	userServer->name = (char*) malloc((15 + 1) * sizeof(char));
	strcpy(userServer->name, userClient->name);

	//send
	if(!serverStop){
		ServerConnect(userServer); //network abstraction

		//receive
		//network abstraction
	}

	//apply changes
	free(userClient->auth); //in best case it's free(NULL)
	userClient->auth = (char*) malloc((26 + 1) * sizeof(char));

	strncpy(userClient->auth, userServer->auth, 26);
	userClient->auth[26] = '\0';
	strncpy(userClient->name, userServer->name, 15);
	userClient->name[15] = '\0';

	//free
	free(userServer->auth);
	free(userServer->character); //in best case it's free(NULL)
	free(userServer->keyS); //in best case it's free(NULL)
	free(userServer->name);
	free(userServer);
}
