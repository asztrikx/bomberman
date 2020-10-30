#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

#include <SDL2/SDL.h>
#include <stdbool.h>

//Position
typedef struct{
	int y, x;
} Position;

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
	Character* owner; //NULL if server or disconnected player
	bool bombOut; //only for ObjectTypeBomb signaling whether player has move out from bomb
} Object;

typedef struct ObjectItem{
	Object object;
	struct ObjectItem* next;
	struct ObjectItem* prev;
} ObjectItem; //Objects may be deleted frequently

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
	Character* character; //handled by characterItem
} UserServer;

typedef struct UserItem{
	UserServer userServer;
	struct UserItem* next;
	struct UserItem* prev;
} UserServerItem;

extern Ability AbilitySpeedExtra;

KeyItem* keyItemSInsert(KeyItem** keyItemS, SDL_Keycode* key);
void keyItemSRemove(KeyItem** keyItemS, KeyItem* keyItem);
void keyItemSFree(KeyItem* keyItemS);
ObjectItem* objectItemSInsert(ObjectItem** objectItemS, Object* object);
void objectItemSInsertItem(ObjectItem** objectItemS, ObjectItem* objectItem);
void objectItemSFree(ObjectItem* objectItemS);
CharacterItem* characterItemSInsert(CharacterItem** characterItemS, Character* character);
void characterItemSInsertItem(CharacterItem** characterItemS, CharacterItem* characterItem);
void characterItemSFree(CharacterItem* characterItemS);
UserServerItem* userServerItemSInsert(UserServerItem** userServerItemS, UserServer* userServer);
void userServerItemSFree(UserServerItem* userServerItemS);

#endif
