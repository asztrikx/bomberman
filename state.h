#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "geometry.h"

//Object
typedef enum{
	ObjectTypeBomb,
	ObjectTypeObstacle,
	ObjectTypeWall,
} ObjectType;

typedef struct{
	Position position;
	ObjectType type;
	long long created; //destroy event => maybe destroyOn
	Position velocity;
} Object;

typedef struct ObjectItem{
	Object* object;
	struct ObjectItem* next;
	struct ObjectItem* prev;
} ObjectItem; //Objects may be deleted frequently

//Key list
typedef struct KeyItem{
	SDL_Keycode key;
	struct KeyItem* next;
	struct KeyItem* prev;
} KeyItem;

//Character
typedef enum{
	CharacterTypeUser,
	CharacterTypeEnemy,
} CharacterType;

typedef struct{
	Position position;
	CharacterType type;
	Position velocity;
	int bomb;
	//Object* bombS; //why?
} Character; //seeable by others

typedef struct CharacterItem{
	Character character;
	struct CharacterItem* next;
	struct CharacterItem* prev;
} CharacterItem; //Characters may be deleted frequently

//World
typedef struct{
	ObjectItem* objectItemS;
	CharacterItem* characterItemS;
	Position* exit; //may not exists (for client)
} World;

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

//User
typedef struct{
	KeyItem* keyItemS; //if NULL then no key was pressed
	char* name; //if NULL?, then no update
	Ability* ablityS;
	char* auth;
	Character* character;
	//??
} User; //not seeable by others

typedef struct UserItem{
	User user;
	struct UserItem* next;
	struct UserItem* prev;
} UserItem;

extern Ability AbilitySpeedExtra;

#endif
