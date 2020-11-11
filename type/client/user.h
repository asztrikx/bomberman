#ifndef TYPE_CLIENT_USER_H_INCLUDED
#define TYPE_CLIENT_USER_H_INCLUDED

#include "../list.h"

typedef struct{
	List* keyList;
	char* name;
	List* ablityList;
	char* auth;
} UserClient;

UserClient* UserClientNew();
void UserClientDelete(UserClient* userClient);

#endif
