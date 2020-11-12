#ifndef TYPE_ANIMATION_H_INCLUDED
#define TYPE_ANIMATION_H_INCLUDED

typedef struct{
	int state;
	int stateDelayTick;
	int stateDelayTickEnd;
} Animation;

Animation AnimationNew(void);

#endif
