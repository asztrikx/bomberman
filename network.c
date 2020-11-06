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
	WorldClient* worldClient = WorldClientNew();

	//exit
	if(worldServer->exit != NULL){
		worldClient->exit = (Position*) malloc(sizeof(Position));
		*(worldClient->exit) = *(worldServer->exit);
	}
	
	//objectS
	worldClient->objectSLength = worldServer->objectList->length;
	worldClient->objectS = (Object*) malloc(worldServer->objectList->length * sizeof(Object));
	ListItem* objectItemCurrent = worldServer->objectList->head;
	for(int i=0; i<worldServer->objectList->length; i++){
		worldClient->objectS[i] = *(Object*)(objectItemCurrent->data);

		objectItemCurrent = objectItemCurrent->next;
	}

	//characterS
	worldClient->characterSLength = worldServer->characterList->length;
	worldClient->characterS = (Character*) malloc(worldServer->characterList->length * sizeof(Character));
	ListItem* listItemCurrent = worldServer->characterList->head;
	for(int i=0; i<worldServer->characterList->length; i++){
		worldClient->characterS[i] = *(Character*)listItemCurrent->data;

		listItemCurrent = listItemCurrent->next;
	}

	//send
	if(!clientStop){ //network abstraction
		ClientReceive(worldClient);
	}

	//free
	WorldClientDelete(worldClient);
}

//networkSendServer send userClient to server as UserServer
void networkSendServer(UserClient* userClient){
	//keyItemS copy
	SDL_Keycode* keyS = (SDL_Keycode*) malloc(userClient->keyList->length * sizeof(SDL_Keycode));
	ListItem* listItemCurrent = userClient->keyList->head;
	for(int i=0; i<userClient->keyList->length; i++){
		keyS[i] = *(SDL_Keycode*)listItemCurrent->data;

		listItemCurrent = listItemCurrent->next;
	}

	//create userServer
	UserServer* userServer = UserServerNew();
	userServer->keyS = keyS;
	userServer->keySLength = userClient->keyList->length;
	
	strcpy(userServer->auth, userClient->auth);
	strcpy(userServer->name, userClient->name);

	//send
	if(!serverStop){
		ServerReceive(userServer); //network abstraction
	}

	//free
	UserServerDelete(userServer);
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
