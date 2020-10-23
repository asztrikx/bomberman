#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include "state.h"

void ServerReceive(User* user);
User* ServerConnect(User* user);
void ServerStart(void);

#endif
