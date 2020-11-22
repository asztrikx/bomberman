#ifndef TYPE_CLIENT_USER_H_INCLUDED
#define TYPE_CLIENT_USER_H_INCLUDED

#include <stdbool.h>
#include "../list.h"
#include "../key.h"

typedef struct{
	bool keyS[KeyLength];
	char* name;
	char* auth;
} UserClient;

UserClient* UserClientNew(void);
void UserClientDelete(UserClient* userClient);

#endif
