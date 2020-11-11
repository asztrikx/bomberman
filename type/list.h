#ifndef TYPE_LIST_H_INCLUDED
#define TYPE_LIST_H_INCLUDED

typedef struct ListItem{
	struct ListItem* next;
	struct ListItem* prev;
	void* data;
} ListItem;

typedef struct{
	ListItem* head;
	int length;
} List;

ListItem* ListInsert(List** list, void* data);
void ListInsertItem(List** list, ListItem* listItem);
void ListRemoveItem(List** list, ListItem* listItem, void (*dataFree)());
ListItem* ListFindItem(List* list, void* data);
List* ListNew();
void ListDelete(List* list, void (*dataFree)());

#endif
