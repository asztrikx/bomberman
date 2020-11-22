#include "../debugmalloc.h"
#include "list.h"
#include <stdlib.h>
#include <stdbool.h>

//ListInsert inserts data into List by reference
ListItem* ListInsert(List** list, void* data){
	ListItem* listItem = (ListItem*) malloc(sizeof(ListItem));
	listItem->data = data;

	ListInsertItem(list, listItem);

	return listItem;
}

//ListInsertItem appends ListItem to List by reference
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

//ListRemoveItem removed ListItem referenced from list
//dataFree is called on ListItem->data if it is not NULL
void ListRemoveItem(List** list, ListItem* listItem, void (*dataFree)(void*)){
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

//ListFindItemByFunction returns first ListItem where func(ListItem->data) holds
ListItem* ListFindItemByFunction(List* list, bool (*func)(void*)){
	for(ListItem* item = list->head; item != NULL; item = item->next){
		if(func(item->data)){
			return item;
		}
	}
	return NULL;
}

void* listFindItemByPointerVariable;

//listFindItemByPointerFunction is a helper function of ListFindItemByPointer
bool listFindItemByPointerFunction(void* data){
	return data == listFindItemByPointerVariable;
}

//ListFindItemByPointer returns first ListItem where ListItem->data == data
//can not be run in parallel
ListItem* ListFindItemByPointer(List* list, void* data){
	listFindItemByPointerVariable = data;
	return ListFindItemByFunction(list, listFindItemByPointerFunction);
}

//ListNew creates a new List
List* ListNew(void){
	List* list = (List*) malloc(sizeof(List));
	list->head = NULL;
	list->length = 0;

	return list;
}

//ListDelete frees all ListItem
//dataFree is called on ListItem->data if it is not NULL
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
