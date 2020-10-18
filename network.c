#include <SDL2/SDL.h>
#include "server.h"
#include "client.h"
#include "state.h"

/*void networkStart(){
	//SDL_AddTimer(1000u/60u, networkTick, NULL);
}*/

void networkSendClient(World* world){
	//encode data
	ClientReceive(world);
}

void networkSendServer(User* user){
	//encode data
	ServerReceive(user);
}

