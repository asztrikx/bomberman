#include "debugmalloc.h"
#include "config.h"
#include "type/geometry.h"

int squaresize = 50;
int velocity = 6;
int velocityEnemy = 2;
int windowHeight = 480;
int windowWidth = 640;
int worldHeight = 9 ;
int worldWidth = 13;
double boxRatio = 0.25;
double enemyRatio = 0.05;
double enemyKeyChangePossibility = 0.015;
const unsigned int tickRate = 1000u/58u;
const long long tickSecond = 1000u/tickRate; //tick count in one second
