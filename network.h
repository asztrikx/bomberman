#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED

#include "state.h"

void networkSendClient(World* world);
void networkSendServer(User* user);
User* networkConnectServer(User* user);

#endif
