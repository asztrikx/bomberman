#ifndef TYPE_SERVER_USER_H_INCLUDED
#define TYPE_SERVER_USER_H_INCLUDED

#include <SDL2/SDL.h>

typedef struct{
	SDL_Keycode* keyS;
	int keySLength;
	char* name; //if NULL then no update
	char* auth;
} UserServer;

UserServer* UserServerNew();
void UserServerDelete(UserServer* userServer);

#endif
