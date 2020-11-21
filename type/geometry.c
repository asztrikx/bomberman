#include "../debugmalloc.h"
#include "geometry.h"
#include <stdbool.h>

//PositionSame determines if a and b have the same coordinate
bool PositionSame(Position a, Position b){
	return a.x == b.x && a.y == b.y;
}
