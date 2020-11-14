#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

#include <SDL2/SDL.h>
#include <stdbool.h>

//[R] delete this

//Ability
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
