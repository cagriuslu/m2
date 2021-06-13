#include "InsertionList.h"
#include <string.h>
#include <assert.h>

int InsertionListInit(InsertionList* list, size_t itemSize, size_t maxItemCount, int (*comparator)(void*, void*)) {
	PROPAGATE_ERROR(ArrayInit(&list->array, itemSize, maxItemCount, maxItemCount));
	list->tmp = malloc(itemSize);
	assert(list->tmp);
	list->comparator = comparator;
	return 0;
}

void InsertionListDeinit(InsertionList* list) {
	ArrayDeinit(&list->array);
	free(list->tmp);
}

size_t InsertionListLength(InsertionList* list) {
	return list->array->length;
}

void* InsertionListGet(InsertionList* list, size_t i) {
	return ArrayGet(&list->array, i);
}

void* InsertionListInsert(InsertionList* list, void* copy) {
	// TODO insert via binary search
	return ArrayAppend(&list->array, copy);
}

void InsertionListSort(InsertionList* list) {
	size_t len = InsertionListLength(list);
	if (1 < len) {
		for (size_t i = 1; i < len; i++) {
			void* currItemPtr = InsertionListGet(list, i);
			memcpy(list->tmp, currItemPtr, list->array->itemSize); // Copy currItem into tmp
			for (size_t j = i; 0 < j--; ) {
				void* iterItemPtr = InsertionListGet(list, j);
				if (0 < list->comparator(currItemPtr, iterItemPtr)) {
					// Copy iter into next item
					memcpy(InsertionListGet(list, j + 1), iterItemPtr, list->array->itemSize);
					// Put curr object in iter's place
					memcpy(iterItemPtr, currItemPtr, list->array->itemSize);
					// TODO this can be optimized
					// Don't copy the item in place every time
				} else {
					break;
				}
			}
		}
	}
}
