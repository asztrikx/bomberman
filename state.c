#include "debugmalloc.h"
#include "state.h"
#include "geometry.h"
#include <stdlib.h>

void intfree(int* a){free(a);}//[R]

void* Copy(void* data, size_t size){
	unsigned char* copy = (unsigned char*) malloc(size * sizeof(char));
	memcpy(copy, data, size);
	return copy;
}

//ListInsert inserts data into list by reference
ListItem* ListInsert(List** list, void* data){
	ListItem* listItem = (ListItem*) malloc(sizeof(ListItem));
	listItem->data = data;

	ListInsertItem(list, listItem);

	return listItem;
}

//ListInsertItem appends listItem to list by reference
void ListInsertItem(List** list, ListItem* listItem){
	if((*list)->head == NULL){ //first element
		listItem->next = NULL;
		listItem->prev = NULL;
	} else { //insert front
		listItem->next = (*list)->head;
		listItem->prev = NULL;

		(*list)->head->prev = listItem;
	}

	(*list)->head = listItem;
	(*list)->length++;
}

void ListRemoveItem(List** list, ListItem* listItem, void (*dataFree)()){
	if(listItem->prev == NULL){ //first in list
		(*list)->head = listItem->next;

		//last in list also
		if(listItem->next != NULL){
			listItem->next->prev = NULL;
		}
	} else if (listItem->next == NULL) { //last in list, but not first
		listItem->prev->next = NULL;
	} else {
		listItem->prev->next = listItem->next;
		listItem->next->prev = listItem->prev;
	}

	//free
	if(dataFree != NULL){
		dataFree(listItem->data);
	}
	free(listItem);
	(*list)->length--;
}

//ListFind return ListItem who's data points to data
ListItem* ListFindItem(List* list, void* data){
	ListItem* listItemCurrent = list->head;
	while(listItemCurrent != NULL){
		if(listItemCurrent->data == data){
			return listItemCurrent;
		}

		listItemCurrent = listItemCurrent->next;
	}

	return NULL;
}

List* ListNew(){
	List* list = (List*) malloc(sizeof(List));
	list->head = NULL;
	list->length = 0;

	return list;
}

//ListDelete frees all ListItem
void ListDelete(List* list, void (*dataFree)()){
	ListItem* listItemCurrent = list->head;
	ListItem* listItemPrev;
	while(listItemCurrent != NULL){
		listItemPrev = listItemCurrent;
		listItemCurrent = listItemCurrent->next;

		if(dataFree != NULL){
			dataFree(listItemPrev->data);
		}
		free(listItemPrev);
	}

	free(list);
}

//UserClientNew
//return must be freed
UserClient* UserClientNew(){
	UserClient* userClient = (UserClient*) malloc(sizeof(UserClient));
	//userClient->ablityList = ListNew();
	userClient->auth = (char*) malloc((26 + 1) * sizeof(char));
	userClient->keyList = ListNew();
	userClient->name = (char*) malloc((15 + 1) * sizeof(char));

	return userClient;
}

void UserClientDelete(UserClient* userClient){
	//ListDelete(userClient->ablityList, );
	free(userClient->auth);
	ListDelete(userClient->keyList, intfree);
	free(userClient->name);
	free(userClient);
}

WorldServer* WorldServerNew(){
	WorldServer* worldServer = (WorldServer*) malloc(sizeof(WorldServer));
	worldServer->characterList = ListNew();
	worldServer->exit = NULL;
	worldServer->height = 0;
	worldServer->objectList = ListNew();
	worldServer->width = 0;

	return worldServer;
}

void WorldServerDelete(WorldServer* worldServer){
	ListDelete(worldServer->characterList, CharacterDelete);
	free(worldServer->exit);
	ListDelete(worldServer->objectList, ObjectDelete);
	free(worldServer);
}

WorldClient* WorldClientNew(){
	WorldClient* worldClient = (WorldClient*) malloc(sizeof(WorldClient));
	worldClient->characterS = NULL;
	worldClient->characterSLength = 0;
	worldClient->exit = NULL;
	worldClient->objectS = NULL;
	worldClient->objectSLength = 0;

	return worldClient;
}

void WorldClientDelete(WorldClient* worldClient){
	free(worldClient->characterS);
	free(worldClient->exit);
	free(worldClient->objectS);
	free(worldClient);
}

UserServer* UserServerNew(){
	UserServer* userServer = (UserServer*) malloc(sizeof(UserServer));
	userServer->auth = (char*) malloc((26 + 1) * sizeof(char));
	userServer->character = NULL;
	userServer->keyS = NULL;
	userServer->keySLength = 0;
	userServer->name = (char*) malloc((15 + 1) * sizeof(char));

	return userServer;
}

void UserServerDelete(UserServer* userServer){
	free(userServer->auth);
	//free(userServer->character); //it is not free'd by this
	free(userServer->keyS);
	free(userServer->name);
	free(userServer);
}

Object* ObjectNew(){
	Object* object = (Object*) malloc(sizeof(Object));
	object->owner = NULL;

	return object;
}

void ObjectDelete(Object* object){
	//free(object->owner) not handled by this
	free(object);
}

Character* CharacterNew(){
	Character* character = (Character*) malloc(sizeof(Character));
	character->owner = NULL;

	return character;
}

void CharacterDelete(Character* character){
	//free(character->owner); not handled by this
	free(character);
}

Ability AbilitySpeedExtra = {
	.speedExtra = 10,
};
