#ifndef INSERTION_LIST_H
#define INSERTION_LIST_H

#include <m2/Def.h>
#include <stdint.h>
#include <vector>

struct InsertionList {
	std::vector<ID> array;
	int (*comparator)(ID, ID);

	InsertionList();
};

int InsertionList_Init(InsertionList* list, size_t maxItemCount, int (*comparator)(ID, ID));
void InsertionList_Term(InsertionList* list);

size_t InsertionList_Length(InsertionList* list);
ID InsertionList_Get(InsertionList* list, size_t i);

void InsertionList_Insert(InsertionList* list, ID id);
void InsertionList_Remove(InsertionList* list, ID id);
void InsertionList_Sort(InsertionList* list);

#endif
