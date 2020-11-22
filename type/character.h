#ifndef TYPE_CHARACTER_H_INCLUDED
#define TYPE_CHARACTER_H_INCLUDED

#include <stdbool.h>
#include "geometry.h"
#include "user/server.h"
#include "animation.h"
#include "key.h"

typedef enum{
	CharacterTypeUser,
	CharacterTypeEnemy,
	CharacterTypeYou,
} CharacterType;

typedef struct{
	Position position;
	CharacterType type;
	int velocity;
	int bombCount; //number of placed bombs for easier check
	UserServer* owner; //NULL if server or disconnected player
	Animation animation;
	bool keyS[KeyLength];
} Character; //seeable by others

Character* CharacterNew(void);
void CharacterDelete(Character* character);

#endif
