#ifndef LIST_H
#define LIST_H

#include "Bucket.h"

typedef struct _List {
	Bucket bucket;
	uint64_t firstId;
	uint64_t lastId;
	size_t dataSize;
} List;

int ListInit(List* list, size_t itemSize);
void ListDeinit(List* list);

void* ListAppend(List* list, void* copy, uint64_t* outIterator);
void* ListPrepend(List* list, void* copy, uint64_t* outIterator);
void* ListInsertAfter(List* list, uint64_t afterIterator, void* copy, uint64_t* outIterator);
void* ListInsertBefore(List* list, uint64_t beforeIterator, void* copy, uint64_t* outIterator);
void ListRemove(List* list, uint64_t iterator);
void ListClear(List* list);

uint64_t ListGetFirst(List* list);
uint64_t ListGetByIndex(List* list, size_t index);
uint64_t ListGetNext(List* list, uint64_t iterator);
uint64_t ListGetPrev(List* list, uint64_t iterator);
uint64_t ListGetLast(List* list);
void* ListGetData(List* list, uint64_t iterator);

#endif
