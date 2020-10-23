#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include "state.h"

void ClientReceive(World* _world);
void ClientStart(void);
void ClientEventKey(SDL_Event sdl_event);

#endif
