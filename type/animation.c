#include "../debugmalloc.h"
#include "animation.h"

//AnimationNew creates a new Animation
Animation AnimationNew(void){
	Animation animation;
	animation.state = 0;
	animation.stateDelayTick = 0;
	animation.stateDelayTickEnd = 2;

	return animation;
}
