#include "debugmalloc.h"
#include "state.h"
#include "geometry.h"
#include <stdlib.h>

Ability AbilitySpeedExtra = {
	.speedExtra = 10,
};

ObjectItem* objectItemSInsert(ObjectItem** objectItemS, Object* object){
	ObjectItem* objectItem = (ObjectItem*) malloc(sizeof(ObjectItem));
	objectItem->object = *object;
	
	if(*objectItemS == NULL){ //first element
		objectItem->next = NULL;
		objectItem->prev = NULL;

		*objectItemS = objectItem;
	} else { //insert front
		objectItem->next = *objectItemS;
		objectItem->prev = NULL;

		(*objectItemS)->prev = objectItem;

		(*objectItemS) = objectItem;
	}

	return objectItem;
}

CharacterItem* characterItemSInsert(CharacterItem** characterItemS, Character* character){
	CharacterItem* characterItem = (CharacterItem*) malloc(sizeof(CharacterItem));
	characterItem->character = *character;
	
	if(*characterItemS == NULL){ //first element
		characterItem->next = NULL;
		characterItem->prev = NULL;

		*characterItemS = characterItem;
	} else { //insert front
		characterItem->next = *characterItemS;
		characterItem->prev = NULL;

		(*characterItemS)->prev = characterItem;

		(*characterItemS) = characterItem;
	}

	return characterItem;
}

UserServerItem* userServerItemSInsert(UserServerItem** userServerItemS, UserServer* userServer){
	UserServerItem* userServerItem = (UserServerItem*) malloc(sizeof(UserServerItem));
	userServerItem->userServer = *userServer;
	
	if(*userServerItemS == NULL){ //first element
		userServerItem->next = NULL;
		userServerItem->prev = NULL;

		*userServerItemS = userServerItem;
	} else { //insert front
		userServerItem->next = *userServerItemS;
		userServerItem->prev = NULL;

		(*userServerItemS)->prev = userServerItem;

		(*userServerItemS) = userServerItem;
	}

	return userServerItem;
}
