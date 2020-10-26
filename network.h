#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED

#include "state.h"

void networkServerStop(void);
void networkServerStart(void);
void networkClientStop(void);
void networkClientStart(void);
void networkSendClient(WorldServer* worldServer);
void networkSendServer(UserClient* userClient);
void networkConnectServer(UserClient* userClient);

#endif
