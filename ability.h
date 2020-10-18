#ifndef ABILITY_H_INCLUDED
#define ABILITY_H_INCLUDED

#include <stdbool.h>

typedef struct{
	int speedExtra;
	int bombCountExtra;
	int bombRadius;
	bool bombRemote;
	bool bombPush;
	bool bombPass;
	bool wallPass;
} Ability;

extern Ability AbilitySpeedExtra;

#endif
