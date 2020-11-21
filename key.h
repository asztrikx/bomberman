#ifndef KEY_H_INCLUDED
#define KEY_H_INCLUDED

#include "type/object.h"
#include "type/character.h"
#include "type/world/server.h"

bool KeyMovementCollisionDetectObject(void* this, Object* that);
bool KeyMovementCollisionDetectCharacter(void* this, Character* that);
void KeyMovement(Character* character, WorldServer* worldServer);
void KeyBombPlace(Character* character, WorldServer* worldServer, long long tickCount);
void KeyMovementRandom(Character* character);

#endif
