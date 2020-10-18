#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#include "geometry.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

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

//World
typedef struct{
	ObjectItem* objectItemS;
	CharacterItem* characterItemS;
	Position* exit; //may not exists (for client)
} World;

//Key list
typedef struct KeyItem{
	SDL_Keysym* key;
	struct KeyItem* next;
	struct KeyItem* prev;
} KeyItem;

//User
typedef struct{
	KeyItem* keyItemS; //if NULL then no key was pressed
	char* name; //if NULL?, then no update
	Ability* ablityS;
	char* id;
	//?
} User; //not seeable by others

typedef struct UserItem{
	User user;
	struct UserItem* next;
	struct UserItem* prev;
} UserItem;

//Character
typedef enum{
	CharacterTypeUser,
	CharacterTypeEnemy,
} CharacterType;

typedef struct{
	Position position;
	CharacterType type;
	char* id;
	//Object* bombS; //why?
} Character; //seeable by others

typedef struct CharacterItem{
	Character* character;
	struct CharacterItem* next;
	struct CharacterItem* prev;
} CharacterItem; //Characters may be deleted frequently

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
