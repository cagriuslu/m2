#ifndef LIST_H
#define LIST_H

#include "Bucket.h"

typedef struct _List {
	Bucket bucket;
	ID firstId;
	ID lastId;
	size_t dataSize;
} List;
typedef List ListOfVec2Is;

int List_Init(List* list, size_t itemSize);
void List_Term(List* list);

void* List_Append(List* list, void* copy, ID* outIterator);
void* List_Prepend(List* list, void* copy, ID* outIterator);
void* List_InsertAfter(List* list, ID afterIterator, void* copy, ID* outIterator);
void* List_InsertBefore(List* list, ID beforeIterator, void* copy, ID* outIterator);
void List_Remove(List* list, ID iterator);
void List_Clear(List* list);

size_t List_Length(List* list);
ID List_GetFirst(List* list);
ID List_GetByIndex(List* list, size_t index);
ID List_GetNext(List* list, ID iterator);
ID List_GetPrev(List* list, ID iterator);
ID List_GetLast(List* list);
void* List_GetData(List* list, ID iterator);

#endif
