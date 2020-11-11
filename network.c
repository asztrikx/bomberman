#include "debugmalloc.h"
#include "network.h"
#include <SDL2/SDL.h>
#include "server.h"
#include "client.h"

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
	int index = 0;
	for(ListItem* item = worldServer->objectList->head; item != NULL; item = item->next, index++){
		worldClient->objectS[index] = *(Object*)(item->data);
	}

	//characterS
	worldClient->characterSLength = worldServer->characterList->length;
	worldClient->characterS = (Character*) malloc(worldServer->characterList->length * sizeof(Character));
	index = 0;
	for(ListItem* item = worldServer->characterList->head; item != NULL; item = item->next, index++){
		worldClient->characterS[index] = *(Character*)(item->data);
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
	int index = 0;
	for(ListItem* item = userClient->keyList->head; item != NULL; item = item->next, index++){
		keyS[index] = *(SDL_Keycode*)item->data;
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
	UserServer* userServer = UserServerNew();
	strcpy(userServer->name, userClient->name);

	//send
	if(!serverStop){
		ServerConnect(userServer); //network abstraction

		//receive
		//network abstraction
	}

	//apply changes
	strncpy(userClient->auth, userServer->auth, 26);
	userClient->auth[26] = '\0';
	strncpy(userClient->name, userServer->name, 15); //name could be occupied
	userClient->name[15] = '\0';

	//free
	UserServerDelete(userServer);
}
