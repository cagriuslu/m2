#include "InsertionList.h"
#include "Bucket.h"
#include <string.h>

#define AsUint64Ptr(ptr) ((ID*) (ptr))

int InsertionListInit(InsertionList* list, size_t maxItemCount, int (*comparator)(ID, ID)) {
	PROPAGATE_ERROR(Array_Init(&list->array, sizeof(ID), maxItemCount, maxItemCount));
	list->comparator = comparator;
	return 0;
}

void InsertionListDeinit(InsertionList* list) {
	Array_Term(&list->array);
}

size_t InsertionListLength(InsertionList* list) {
	return list->array.length;
}

ID InsertionListGet(InsertionList* list, size_t i) {
	ID* ptr = Array_Get(&list->array, i);
	return ptr ? *ptr : 0;
}

void InsertionListInsert(InsertionList* list, ID id) {
	// TODO insert via binary search
	Array_Append(&list->array, &id);
}

void InsertionListRemove(InsertionList* list, ID id) {
	// TODO find via binary search
	for (size_t i = 0; i < list->array.length; i++) {
		ID* ptr = Array_Get(&list->array, i);
		if (ptr && id == *ptr) {
			Array_Remove(&list->array, i);
			return;
		}
	}
}

void InsertionListSort(InsertionList* list) {
	for (size_t i = 1; i < list->array.length; i++) {
		ID currItem = InsertionListGet(list, i);
		for (size_t j = i; 0 < j--; ) {
			ID iterItem = InsertionListGet(list, j);
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
