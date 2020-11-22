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
ListItem* ListFindItemByFunction(List* list, bool (*func)(void*));
ListItem* ListFindItemByPointer(List* list, void* data);
List* ListNew(void);
void ListDelete(List* list, void (*dataFree)());

#endif
