#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED

#include "state.h"

void networkSendServer(User* user);
void networkSendClient(World* world);
void networkConnectClient(User* user);
void networkConnectServer(User* user);

#endif
