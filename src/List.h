#ifndef LIST_H
#define LIST_H

#include "Bucket.h"

typedef struct _List {
	Bucket bucket;
	ID firstId;
	ID lastId;
	size_t dataSize;
} List;
typedef List ListOfVec2I;

int ListInit(List* list, size_t itemSize);
void ListDeinit(List* list);

void* ListAppend(List* list, void* copy, ID* outIterator);
void* ListPrepend(List* list, void* copy, ID* outIterator);
void* ListInsertAfter(List* list, ID afterIterator, void* copy, ID* outIterator);
void* ListInsertBefore(List* list, ID beforeIterator, void* copy, ID* outIterator);
void ListRemove(List* list, ID iterator);
void ListClear(List* list);

size_t ListLength(List* list);
ID ListGetFirst(List* list);
ID ListGetByIndex(List* list, size_t index);
ID ListGetNext(List* list, ID iterator);
ID ListGetPrev(List* list, ID iterator);
ID ListGetLast(List* list);
void* ListGetData(List* list, ID iterator);

#endif
