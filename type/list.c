#include "../debugmalloc.h"
#include "list.h"
#include <stdlib.h>

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

//ListFind returns first ListItem where ListItem->data == data
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

//ListNew creates a new List
//it must be freed by caller
List* ListNew(){
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
