#include "debugmalloc.h"
#include "state.h"
#include "geometry.h"
#include <stdlib.h>

KeyItem* keyItemSInsert(KeyItem** keyItemS, SDL_Keycode* key){
	KeyItem* keyItem = (KeyItem*) malloc(sizeof(KeyItem));
	keyItem->key = *key;

	if(*keyItemS == NULL){ //first element
		keyItem->next = NULL;
		keyItem->prev = NULL;

		*keyItemS = keyItem;
	} else { //insert front
		keyItem->next = *keyItemS;
		keyItem->prev = NULL;

		(*keyItemS)->prev = keyItem;

		(*keyItemS) = keyItem;
	}

	return keyItem;
}

void keyItemSRemove(KeyItem** keyItemS, KeyItem* keyItem){
	if(keyItem->prev == NULL){ //first in list
		*keyItemS = keyItem->next;

		//last in list also
		if(keyItem->next != NULL){
			keyItem->next->prev = NULL;
		}
	} else if (keyItem->next == NULL) { //last in list, but not first
		keyItem->prev->next = NULL;
	} else {
		keyItem->prev->next = keyItem->next;
		keyItem->next->prev = keyItem->prev;
	}

	//free
	free(keyItem);
}

void keyItemSFree(KeyItem* keyItemS){
	KeyItem* keyItemCurrent = keyItemS;
	KeyItem* keyItemPrev;
	while(keyItemCurrent != NULL){
		keyItemPrev = keyItemCurrent;
		keyItemCurrent = keyItemCurrent->next;

		free(keyItemPrev);
	}
}

Ability AbilitySpeedExtra = {
	.speedExtra = 10,
};

ObjectItem* objectItemSInsert(ObjectItem** objectItemS, Object* object){
	ObjectItem* objectItem = (ObjectItem*) malloc(sizeof(ObjectItem));
	Object* objectCopy = (Object*) malloc(sizeof(Object));
	*objectCopy = *object;
	objectItem->object = objectCopy;
	
	objectItemSInsertItem(objectItemS, objectItem);

	return objectItem;
}

void objectItemSInsertItem(ObjectItem** objectItemS, ObjectItem* objectItem){
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
}

ObjectItem* objectItemSFind(ObjectItem* objectItemS, Object* object){
	ObjectItem* objectItemCurrent = objectItemS;
	while(objectItemCurrent != NULL){
		if(objectItemCurrent->object == object){
			return objectItemCurrent;
		}

		objectItemCurrent = objectItemCurrent->next;
	}

	return NULL;
}

void objectItemSRemove(ObjectItem** objectItemS, ObjectItem* objectItem, bool objectFree){
	//relink
	if(objectItem->prev == NULL && objectItem->next == NULL){
		*objectItemS = NULL;
	} else if(objectItem->prev == NULL){
		objectItem->next->prev = NULL;

		*objectItemS = objectItem->next;
	} else if(objectItem->next == NULL){
		objectItem->prev->next = NULL;
	} else {
		objectItem->prev->next = objectItem->next;
		objectItem->next->prev = objectItem->prev;
	}

	//free
	if(objectFree){
		free(objectItem->object);
	}
	free(objectItem);
}

void objectItemSFree(ObjectItem* objectItemS, bool objectFree){
	ObjectItem* objectItemCurrent = objectItemS;
	ObjectItem* objectItemPrev;
	while(objectItemCurrent != NULL){
		objectItemPrev = objectItemCurrent;
		objectItemCurrent = objectItemCurrent->next;

		if(objectFree){
			free(objectItemPrev->object);
		}
		free(objectItemPrev);
	}
}

CharacterItem* characterItemSInsert(CharacterItem** characterItemS, Character* character){
	CharacterItem* characterItem = (CharacterItem*) malloc(sizeof(CharacterItem));
	Character* characterCopy = (Character*) malloc(sizeof(Character));
	*characterCopy = *character;
	characterItem->character = characterCopy;
	
	characterItemSInsertItem(characterItemS, characterItem);

	return characterItem;
}

void characterItemSInsertItem(CharacterItem** characterItemS, CharacterItem* characterItem){
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
}

CharacterItem* characterItemSFind(CharacterItem* characterItemS, Character* character){
	CharacterItem* characterItemCurrent = characterItemS;
	while(characterItemCurrent != NULL){
		if(characterItemCurrent->character == character){
			return characterItemCurrent;
		}

		characterItemCurrent = characterItemCurrent->next;
	}

	return NULL;
}

void characterItemSRemove(CharacterItem** characterItemS, CharacterItem* characterItem, bool characterFree){
	//relink
	if(characterItem->prev == NULL && characterItem->next == NULL){
		*characterItemS = NULL;
	} else if(characterItem->prev == NULL){
		characterItem->next->prev = NULL;

		*characterItemS = characterItem->next;
	} else if(characterItem->next == NULL){
		characterItem->prev->next = NULL;
	} else {
		characterItem->prev->next = characterItem->next;
		characterItem->next->prev = characterItem->prev;
	}

	//free
	if(characterFree){
		free(characterItem->character);
	}
	free(characterItem);
}

void characterItemSFree(CharacterItem* characterItemS, bool characterFree){
	CharacterItem* characterItemCurrent = characterItemS;
	CharacterItem* characterItemPrev;
	while(characterItemCurrent != NULL){
		characterItemPrev = characterItemCurrent;
		characterItemCurrent = characterItemCurrent->next;

		if(characterFree){
			free(characterItemPrev->character);
		}
		free(characterItemPrev);
	}
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

void userServerItemSFree(UserServerItem* userServerItemS){
	UserServerItem* userServerItemCurrent = userServerItemS;
	UserServerItem* userServerItemPrev;
	while(userServerItemCurrent != NULL){
		userServerItemPrev = userServerItemCurrent;
		userServerItemCurrent = userServerItemCurrent->next;

		free(userServerItemPrev->userServer.auth);
		//free(userServerItemPrev->userServer.character); //it is free'd by characterItemSFree
		free(userServerItemPrev->userServer.keyS);
		free(userServerItemPrev->userServer.name);
		free(userServerItemPrev);
	}
}
