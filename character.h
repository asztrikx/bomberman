#ifndef CHARACTER_H_INCLUDED
#define CHARACTER_H_INCLUDED

#include "ability.h"
#include "object.h"
#include "geometry.h"

typedef enum{
	CharacterTypeUser,
	CharacterTypeEnemy,
} CharacterType;

typedef struct{
	Position position;
	Ability* ablityS;
	CharacterType type;
	//pressed buttons
	//Object* bombS; //why?
} Character;

#endif
