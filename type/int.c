#include "../debugmalloc.h"
#include "int.h"
#include <stdlib.h>

//ObjectNew creates a new Object
int* IntNew(){
	int* i = (int*) malloc(sizeof(int));
	return i;
}

//IntDelete frees int
void IntDelete(int* a){
	free(a);
}
