#include "InsertionList.h"
#include "Pool.h"
#include <string.h>

#define AsUint64Ptr(ptr) ((ID*) (ptr))

int InsertionList_Init(InsertionList* list, size_t maxItemCount, int (*comparator)(ID, ID)) {
	memset(list, 0, sizeof(InsertionList));
	XERR_REFLECT(Array_Init(&list->array, sizeof(ID), maxItemCount, maxItemCount, NULL));
	list->comparator = comparator;
	return 0;
}

void InsertionList_Term(InsertionList* list) {
	Array_Term(&list->array);
}

size_t InsertionList_Length(InsertionList* list) {
	return list->array.length;
}

ID InsertionList_Get(InsertionList* list, size_t i) {
	ID* ptr = Array_Get(&list->array, i);
	return ptr ? *ptr : 0;
}

void InsertionList_Insert(InsertionList* list, ID id) {
	// TODO insert via binary search
	Array_Append(&list->array, &id);
}

void InsertionList_Remove(InsertionList* list, ID id) {
	// TODO find via binary search
	for (size_t i = 0; i < list->array.length; i++) {
		ID* ptr = Array_Get(&list->array, i);
		if (ptr && id == *ptr) {
			Array_Remove(&list->array, i);
			return;
		}
	}
}

void InsertionList_Sort(InsertionList* list) {
	for (size_t i = 1; i < list->array.length; i++) {
		ID currItem = InsertionList_Get(list, i);
		for (size_t j = i; 0 < j--; ) {
			ID iterItem = InsertionList_Get(list, j);
			if (0 < list->comparator(currItem, iterItem)) {
				// Copy iter into next item
				*AsUint64Ptr(Array_Get(&list->array, j + 1)) = iterItem;
				// Put curr object in place of iter
				*AsUint64Ptr(Array_Get(&list->array, j)) = currItem;
			} else {
				break;
			}
		}
	}
}
