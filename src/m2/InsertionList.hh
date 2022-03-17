#ifndef INSERTION_LIST_H
#define INSERTION_LIST_H

#include "m2/Array.hh"
#include "Pool.hh"
#include <stdint.h>

typedef struct _InsertionList {
	// TODO deque might be faster
	Array array;
	int (*comparator)(ID, ID);
} InsertionList;

int InsertionList_Init(InsertionList* list, size_t maxItemCount, int (*comparator)(ID, ID));
void InsertionList_Term(InsertionList* list);

size_t InsertionList_Length(InsertionList* list);
ID InsertionList_Get(InsertionList* list, size_t i);

void InsertionList_Insert(InsertionList* list, ID id);
void InsertionList_Remove(InsertionList* list, ID id);
void InsertionList_Sort(InsertionList* list);

#endif
