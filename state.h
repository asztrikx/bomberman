#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

#include <SDL2/SDL.h>
#include <stdbool.h>

//Position
typedef struct{
	int y, x;
} Position;

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
	Object object;
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
	CharacterTypeYou,
} CharacterType;

typedef struct{
	Position position;
	CharacterType type;
	Position velocity;
	int bomb;
	//Object* bombS; //why?
	char* name;
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
	Position* exit;
} WorldServer;

typedef struct{
	Object* objectS;
	int objectSLength;
	Character* characterS;
	int characterSLength;
	Position* exit; //may not exists (for client)
} WorldClient;

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
	KeyItem* keyItemS;
	char* name;
	Ability* ablityS;
	char* auth;
} UserClient;

typedef struct{
	SDL_Keycode* keyS;
	int keySLength;
	char* name; //if NULL then no update
	char* auth;
	Character* character;
} UserServer;

typedef struct UserItem{
	UserServer userServer;
	struct UserItem* next;
	struct UserItem* prev;
} UserServerItem;

extern Ability AbilitySpeedExtra;

ObjectItem* objectItemSInsert(ObjectItem** objectItemS, Object* object);
CharacterItem* characterItemSInsert(CharacterItem** characterItemS, Character* character);
UserServerItem* userServerItemSInsert(UserServerItem** userServerItemS, UserServer* userServer);

#endif
