#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED

#include "state.h"

extern int squaresize;
extern Position velocity;
extern int windowHeight;
extern int windowWidth;

bool collisionPositionS(Position position1, Position position2);
bool collisionObjectS(ObjectItem* objectItems, Position position);
bool collisionCharacterS(CharacterItem* characterItemS, Position position, Character* characterException);
WorldServer* worldGenerate(int height, int width);

#endif
