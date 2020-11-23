#include "../debugmalloc.h"
#include "character.h"
#include <stdlib.h>
#include "key.h"

//CharacterNew creates a new Character
Character* CharacterNew(void){
	Character* character = (Character*) malloc(sizeof(Character));
	character->animation = (Animation){
		.state = 0,
		.stateDelayTick = 0,
		.stateDelayTickEnd = 10,
	};
	character->bombCount = 0;
	for(int i=0; i<KeyLength; i++){
		character->keyS[i] = false;
	}
	character->owner = NULL;
	character->position = (Position){
		.y = 0,
		.x = 0,
	};
	character->velocity = 0;
	return character;
}

//CharacterDelete frees Character
void CharacterDelete(Character* character){
	free(character);
}
