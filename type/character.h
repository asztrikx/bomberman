#ifndef TYPE_CHARACTER_H_INCLUDED
#define TYPE_CHARACTER_H_INCLUDED

#include "geometry.h"
#include "user/server.h"

typedef enum{
	CharacterTypeUser,
	CharacterTypeEnemy,
	CharacterTypeYou,
} CharacterType;

typedef struct{
	Position position;
	CharacterType type;
	Position velocity;
	int bombCount; //number of placed bombs for easier check
	UserServer* owner; //NULL if server or disconnected player
} Character; //seeable by others

Character* CharacterNew();
void CharacterDelete(Character* character);

#endif
