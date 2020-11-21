#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include "type/user/server.h"

void ServerReceive(UserServer* userServerUnsafe);
void ServerConnect(UserServer* userServerUnsafe);
void ServerStart(bool load);
void ServerStop(void);

#endif
