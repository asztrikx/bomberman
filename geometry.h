#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED

#include "state.h"

extern int squaresize;
extern Position velocity;
extern int windowHeight;
extern int windowWidth;

bool collisionPoint(Position position1, Position position2);
bool collisionLine(Position from, Position to, Position obstacle);
List* collisionObjectS(List* list, Position from, Position to);
List* collisionCharacterS(List* list, Position from, Position to);
WorldServer* worldGenerate(int height, int width);

#endif
