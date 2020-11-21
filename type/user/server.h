#ifndef TYPE_SERVER_USER_H_INCLUDED
#define TYPE_SERVER_USER_H_INCLUDED

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "../key.h"
#include "../gamestate.h"

typedef struct{
	bool keyS[KeyLength];
	char* name; //if NULL then no update
	char* auth;
	Gamestate gamestate;
} UserServer;

UserServer* UserServerNew();
void UserServerDelete(UserServer* userServer);

#endif
