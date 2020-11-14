#include "../debugmalloc.h"
#include "character.h"
#include <stdlib.h>

//CharacterNew creates a new Character
Character* CharacterNew(){
	Character* character = (Character*) malloc(sizeof(Character));
	character->animation = (Animation){
		.state = 0,
		.stateDelayTick = 0,
		.stateDelayTickEnd = 0,
	};
	character->owner = NULL;

	return character;
}

//CharacterDelete frees Character
void CharacterDelete(Character* character){
	free(character);
}
