#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED

#include "type/user/client.h"
#include "type/world/server.h"

void networkServerStop(void);
void networkServerStart(void);
void networkClientStop(void);
void networkClientStart(void);
void networkSendClient(WorldServer* worldServer, UserServer* userServer);
void networkSendServer(UserClient* userClient);
void networkConnectServer(UserClient* userClient);

#endif
