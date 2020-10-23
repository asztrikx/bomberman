#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include "state.h"

void ClientReceive(WorldClient* worldCopy);
void ClientStart(void);
void ClientEventKey(SDL_Event sdl_event);
void ClientConnect(void);

#endif
