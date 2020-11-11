#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include "type/server/user.h"

void ServerReceive(UserServer* userServerUnsafe);
void ServerConnect(UserServer* userServerUnsafe);
void ServerStart(void);
void ServerStop(void);

#endif
