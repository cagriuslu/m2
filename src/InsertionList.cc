#include "m2/InsertionList.hh"

#define AsUint64Ptr(ptr) ((ID*) (ptr))

InsertionList::InsertionList() : comparator(nullptr) {
}

int InsertionList_Init(InsertionList* list, size_t maxItemCount, int (*comparator)(ID, ID)) {
	list->comparator = comparator;
	return 0;
}

void InsertionList_Term(InsertionList* list) {
	list->array.clear();
}

size_t InsertionList_Length(InsertionList* list) {
	return list->array.size();
}

ID InsertionList_Get(InsertionList* list, size_t i) {
	return list->array[i];
}

void InsertionList_Insert(InsertionList* list, ID id) {
	// TODO insert via binary search
	list->array.emplace_back(id);
}

void InsertionList_Remove(InsertionList* list, ID id) {
	// TODO find via binary search
	for (size_t i = 0; i < list->array.size(); i++) {
		if (id == list->array[i]) {
			list->array.erase(list->array.begin() + i);
			return;
		}
	}
}

void InsertionList_Sort(InsertionList* list) {
	// TODO employ binary search?
	for (size_t i = 1; i < list->array.size(); i++) {
		ID currItem = InsertionList_Get(list, i);
		for (size_t j = i; 0 < j--; ) {
			ID iterItem = InsertionList_Get(list, j);
			if (0 < list->comparator(currItem, iterItem)) {
				// Copy iter into next item
				list->array[j + 1] = iterItem;
				// Put curr object in place of iter
				list->array[j] = currItem;
			} else {
				break;
			}
		}
	}
}
