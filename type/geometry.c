#include "../debugmalloc.h"
#include "geometry.h"
#include <stdbool.h>

bool PositionSame(Position a, Position b){
	return a.x == b.x && a.y == b.y;
}
