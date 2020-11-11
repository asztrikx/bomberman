#include "../debugmalloc.h"
#include "character.h"
#include <stdlib.h>

//CharacterNew creates a new Character
Character* CharacterNew(){
	Character* character = (Character*) malloc(sizeof(Character));

	return character;
}

//CharacterDelete frees Character
void CharacterDelete(Character* character){
	free(character);
}
