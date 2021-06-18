#ifndef INSERTION_LIST_H
#define INSERTION_LIST_H

#include "Array.h"
#include <stdint.h>

typedef struct _InsertionList {
	Array array;
	int (*comparator)(uint32_t, uint32_t);
} InsertionList;

int InsertionListInit(InsertionList* list, size_t maxItemCount, int (*comparator)(uint32_t, uint32_t));
void InsertionListDeinit(InsertionList* list);

size_t InsertionListLength(InsertionList* list);
uint32_t InsertionListGet(InsertionList* list, size_t i);

void InsertionListInsert(InsertionList* list, uint32_t id);
void InsertionListRemove(InsertionList* list, uint32_t id);
void InsertionListSort(InsertionList* list);

#endif
