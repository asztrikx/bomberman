#ifndef TYPE_ARRAY_H_INCLUDED
#define TYPE_ARRAY_H_INCLUDED

typedef struct{
	int length;
	int capacity;
	void** data;
} Array;

Array* ArrayNew();
void ArrayInsert(Array* array, void* value, size_t size);
void ArrayDelete(Array* array, void(*dataFree)(void*));

#endif
