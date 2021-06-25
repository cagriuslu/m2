#include "List.h"
#include "Error.h"
#include <string.h>

typedef struct _ListItem {
	uint64_t nextId;
	uint64_t prevId;
	char data[0];
} ListItem;

int ListInit(List* list, size_t itemSize) {
	memset(list, 0, sizeof(List));
	PROPAGATE_ERROR(BucketInit(&list->bucket, sizeof(ListItem) + itemSize));
	list->dataSize = itemSize;
	return 0;
}

void ListDeinit(List* list) {
	BucketDeinit(&list->bucket);
	memset(list, 0, sizeof(List));
}

void* ListAppend(List* list, void* copy, uint64_t* outIterator) {
	return ListInsertAfter(list, list->lastId, copy, outIterator);
}

void* ListPrepend(List* list, void* copy, uint64_t* outIterator) {
	return ListInsertAfter(list, 0, copy, outIterator);
}

void* ListInsertAfter(List* list, uint64_t afterIterator, void* copy, uint64_t* outIterator) {
	if (afterIterator == 0) {
		if (list->firstId == 0) {
			uint64_t newItemId = 0;
			ListItem* newItem = BucketMark(&list->bucket, NULL, &newItemId);
			newItem->nextId = 0;
			newItem->prevId = 0;
			if (copy) {
				memcpy(newItem->data, copy, list->dataSize);
			}
			if (outIterator) {
				*outIterator = newItemId;
			}
			list->firstId = newItemId;
			list->lastId = newItemId;
			return newItem->data;
		} else {
			uint64_t oldFirstItemId = list->firstId;
			ListItem* oldFirstItem = BucketGetById(&list->bucket, oldFirstItemId);
			uint64_t newFirstItemId = 0;
			ListItem* newFirstItem = BucketMark(&list->bucket, NULL, &newFirstItemId);
			oldFirstItem->prevId = newFirstItemId;
			newFirstItem->nextId = oldFirstItemId;
			newFirstItem->prevId = 0;
			if (copy) {
				memcpy(newFirstItem->data, copy, list->dataSize);
			}
			if (outIterator) {
				*outIterator = newFirstItemId;
			}
			list->firstId = newFirstItemId;
			return newFirstItem->data;
		}
	} else {
		ListItem* iteratorItem = BucketGetById(&list->bucket, afterIterator);
		if (!iteratorItem) {
			return NULL;
		}
		if (iteratorItem->nextId == 0) {
			uint64_t newLastItemId = 0;
			ListItem* newLastItem = BucketMark(&list->bucket, NULL, &newLastItemId);
			iteratorItem->nextId = newLastItemId;
			newLastItem->prevId = afterIterator;
			newLastItem->nextId = 0;
			if (copy) {
				memcpy(newLastItem->data, copy, list->dataSize);
			}
			if (outIterator) {
				*outIterator = newLastItemId;
			}
			list->lastId = newLastItemId;
			return newLastItem->data;
		} else {
			uint64_t nextItemId = iteratorItem->nextId;
			ListItem* nextItem = BucketGetById(&list->bucket, nextItemId);
			uint64_t newItemId = 0;
			ListItem* newItem = BucketMark(&list->bucket, NULL, &newItemId);
			iteratorItem->nextId = newItemId;
			nextItem->prevId = newItemId;
			newItem->prevId = afterIterator;
			newItem->nextId = nextItemId;
			if (copy) {
				memcpy(newItem->data, copy, list->dataSize);
			}
			if (outIterator) {
				*outIterator = newItemId;
			}
			return newItem->data;
		}
	}
}

void* ListInsertBefore(List* list, uint64_t beforeIterator, void* copy, uint64_t* outIterator) {
	if (beforeIterator == 0) {
		return ListInsertAfter(list, list->lastId, copy, outIterator);
	} else {
		ListItem* iteratorItem = BucketGetById(&list->bucket, beforeIterator);
		if (!iteratorItem) {
			return NULL;
		}
		return ListInsertAfter(list, iteratorItem->prevId, copy, outIterator);
	}
}

void ListRemove(List* list, uint64_t iterator) {
	ListItem* item = BucketGetById(&list->bucket, iterator);
	if (!item) {
		return;
	}
	
	if (iterator == list->firstId && iterator == list->lastId) {
		list->firstId = 0;
		list->lastId = 0;
	} else if (iterator == list->firstId) {
		list->firstId = item->nextId;
		((ListItem*)BucketGetById(&list->bucket, item->nextId))->prevId = 0;
	} else if (iterator == list->lastId) {
		list->lastId = item->prevId;
		((ListItem*)BucketGetById(&list->bucket, item->prevId))->nextId = 0;
	} else {
		((ListItem*)BucketGetById(&list->bucket, item->nextId))->prevId = item->prevId;
		((ListItem*)BucketGetById(&list->bucket, item->prevId))->nextId = item->nextId;
	}
	BucketUnmarkById(&list->bucket, iterator);
}

void ListClear(List* list) {
	BucketUnmarkAll(&list->bucket);
	list->firstId = 0;
	list->lastId = 0;
}

uint64_t ListGetFirst(List* list) {
	return list->firstId;
}

uint64_t ListGetByIndex(List* list, size_t index) {
	uint64_t iterator = list->firstId;
	for (size_t i = 0; i < index; i++) {
		ListItem* item = BucketGetById(&list->bucket, iterator);
		if (item) {
			iterator = item->nextId;
		} else {
			return 0;
		}
	}
	return iterator;
}

uint64_t ListGetNext(List* list, uint64_t iterator) {
	ListItem* item = BucketGetById(&list->bucket, iterator);
	return item ? item->nextId : 0;
}

uint64_t ListGetPrev(List* list, uint64_t iterator) {
	ListItem* item = BucketGetById(&list->bucket, iterator);
	return item ? item->prevId : 0;
}

uint64_t ListGetLast(List* list) {
	return list->lastId;
}

void* ListGetData(List* list, uint64_t iterator) {
	ListItem* item = BucketGetById(&list->bucket, iterator);
	return item ? item->data : NULL;
}
