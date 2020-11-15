#include "debugmalloc.h"
#include "config.h"
#include "type/geometry.h"

int squaresize = 50;
int velocity = 10;
int windowHeight = 480;
int windowWidth = 640;
double boxRatio = 0.4;
double enemyRatio = 0.2;
const unsigned int tickRate = 1000u/58u;
const long long tickSecond = 1000u/tickRate; //tick count in one second
