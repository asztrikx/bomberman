#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED

#include "type/user/client.h"
#include "type/world/server.h"

void NetworkServerStop(void);
void NetworkServerStart(void);
void NetworkClientStop(void);
void NetworkClientStart(void);
void NetworkSendClient(WorldServer* worldServer, UserServer* userServer);
void NetworkSendServer(UserClient* userClient);
void NetworkConnectServer(UserClient* userClient);

#endif
