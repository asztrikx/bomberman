#include "network.h"
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

//networkConnectServer client request to server to create connection
User* networkConnectServer(User* user){
	return ServerConnect(user);
}

