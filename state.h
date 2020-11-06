#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

#include <SDL2/SDL.h>
#include <stdbool.h>

//List
typedef struct ListItem{
	struct ListItem* next;
	struct ListItem* prev;
	void* data;
} ListItem;

typedef struct{
	ListItem* head;
	int length;
} List;

//Position
typedef struct{
	int y, x;
} Position;

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
	int bombCount; //number of placed bombs for easier check
	//Object* bombS; //why?
	char* name; //not handled by this
} Character; //seeable by others

//Object
typedef enum{
	ObjectTypeBomb,
	ObjectTypeBombFire,
	ObjectTypeObstacle,
	ObjectTypeWall,
	ObjectTypeBox,
} ObjectType;

typedef struct{
	Position position;
	ObjectType type;
	long long created;
	long long destroy; //-1 means never gets destroyed by server
	Position velocity;
	Character* owner; //NULL if server or disconnected player
	bool bombOut; //only for ObjectTypeBomb signaling whether player has move out from bomb
} Object;

//World
typedef struct{
	List* objectList;
	List* characterList;
	Position* exit;
	int height, width;
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
	List* keyList;
	char* name;
	List* ablityList; //unused
	char* auth;
} UserClient;

typedef struct{
	SDL_Keycode* keyS;
	int keySLength;
	char* name; //if NULL then no update
	char* auth;
	Character* character; //not handled by this
} UserServer;

extern Ability AbilitySpeedExtra;
void intfree(int* a);
void* Copy(void* data, size_t size);

ListItem* ListInsert(List** list, void* data);
void ListInsertItem(List** list, ListItem* listItem);
void ListRemoveItem(List** list, ListItem* listItem, void (*dataFree)());
ListItem* ListFindItem(List* list, void* data);
List* ListNew();
void ListDelete(List* list, void (*dataFree)());

UserClient* UserClientNew();
void UserClientDelete(UserClient* userClient);

WorldServer* WorldServerNew();
void WorldServerDelete(WorldServer* worldServer);

WorldClient* WorldClientNew();
void WorldClientDelete(WorldClient* worldClient);

void ObjectDelete(Object* object);

void CharacterDelete(Character* character);

UserServer* UserServerNew();
void UserServerDelete(UserServer* userServer);

#endif
