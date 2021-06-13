#ifndef INSERTION_LIST_H
#define INSERTION_LIST_H

#include "Array.h"

typedef struct _InsertionList {
	Array* array;
	int (*comparator)(void*, void*);
	void* tmp;
} InsertionList;

int InsertionListInit(InsertionList* list, size_t itemSize, size_t maxItemCount, int (*comparator)(void*, void*));
void InsertionListDeinit(InsertionList* list);

size_t InsertionListLength(InsertionList* list);
void* InsertionListGet(InsertionList* list, size_t i);

void* InsertionListInsert(InsertionList* list, void* copy);
void InsertionListSort(InsertionList* list);

#endif
