#ifndef INSERTION_LIST_H
#define INSERTION_LIST_H

#include "Array.h"
#include "Bucket.h"
#include <stdint.h>

typedef struct _InsertionList {
	Array array;
	int (*comparator)(ID, ID);
} InsertionList;

int InsertionListInit(InsertionList* list, size_t maxItemCount, int (*comparator)(ID, ID));
void InsertionListDeinit(InsertionList* list);

size_t InsertionListLength(InsertionList* list);
ID InsertionListGet(InsertionList* list, size_t i);

void InsertionListInsert(InsertionList* list, ID id);
void InsertionListRemove(InsertionList* list, ID id);
void InsertionListSort(InsertionList* list);

#endif
