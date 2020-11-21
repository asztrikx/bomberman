#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include "type/world/client.h"

void ClientConnect(void);
void ClientReceive(WorldClient* worldCopy);
void ClientStart(void);
void ClientStop(void);

#endif
