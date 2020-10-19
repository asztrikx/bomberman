#include <SDL2/SDL.h>
#include "server.h"
#include "client.h"
#include "state.h"

void networkSendClient(World* world){
	ClientReceive(world);
}

void networkSendServer(User* user){
	ServerReceive(user);
}

//networkConnectClient server accepting request to create connection
void networkConnectServer(User* user){
	ServerConnect(user);
}

//networkConnectClient client request to server to create connection
void networkConnectClient(User* user){
	networkConnectServer(user);
}

