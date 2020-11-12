#include "../debugmalloc.h"
#include "animation.h"

Animation AnimationNew(void){
	Animation animation;
	animation.state = 0;
	animation.stateDelayTick = 0;
	animation.stateDelayTickEnd = 0;

	return animation;
}
