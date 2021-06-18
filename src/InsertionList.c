#include "InsertionList.h"
#include <string.h>

#define AsUint32Ptr(ptr) ((uint32_t*) (ptr))

int InsertionListInit(InsertionList* list, size_t maxItemCount, int (*comparator)(uint32_t, uint32_t)) {
	PROPAGATE_ERROR(ArrayInit(&list->array, sizeof(uint32_t), maxItemCount, maxItemCount));
	list->comparator = comparator;
	return 0;
}

void InsertionListDeinit(InsertionList* list) {
	ArrayDeinit(&list->array);
}

size_t InsertionListLength(InsertionList* list) {
	return list->array.length;
}

uint32_t InsertionListGet(InsertionList* list, size_t i) {
	uint32_t* ptr = ArrayGet(&list->array, i);
	return ptr ? *ptr : 0;
}

void InsertionListInsert(InsertionList* list, uint32_t id) {
	// TODO insert via binary search
	ArrayAppend(&list->array, &id);
}

void InsertionListRemove(InsertionList* list, uint32_t id) {
	// TODO find via binary search
	for (size_t i = 0; i < list->array.length; i++) {
		uint32_t* ptr = ArrayGet(&list->array, i);
		if (ptr && id == *ptr) {
			ArrayRemove(&list->array, i);
			return;
		}
	}
}

void InsertionListSort(InsertionList* list) {
	for (size_t i = 1; i < list->array.length; i++) {
		uint32_t currItem = InsertionListGet(list, i);
		for (size_t j = i; 0 < j--; ) {
			uint32_t iterItem = InsertionListGet(list, j);
			if (0 < list->comparator(currItem, iterItem)) {
				// Copy iter into next item
				* AsUint32Ptr(ArrayGet(&list->array, j + 1)) = iterItem;
				// Put curr object in place of iter
				*AsUint32Ptr(ArrayGet(&list->array, j)) = currItem;
			} else {
				break;
			}
		}
	}
}
