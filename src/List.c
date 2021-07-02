#include "List.h"
#include "Error.h"
#include <string.h>

typedef struct _ListItem {
	ID nextId;
	ID prevId;
	char data[0];
} ListItem;

int ListInit(List* list, size_t itemSize) {
	memset(list, 0, sizeof(List));
	PROPAGATE_ERROR(Bucket_Init(&list->bucket, sizeof(ListItem) + itemSize));
	list->dataSize = itemSize;
	return 0;
}

void ListDeinit(List* list) {
	Bucket_Term(&list->bucket);
	memset(list, 0, sizeof(List));
}

void* ListAppend(List* list, void* copy, ID* outIterator) {
	return ListInsertAfter(list, list->lastId, copy, outIterator);
}

void* ListPrepend(List* list, void* copy, ID* outIterator) {
	return ListInsertAfter(list, 0, copy, outIterator);
}

void* ListInsertAfter(List* list, ID afterIterator, void* copy, ID* outIterator) {
	if (afterIterator == 0) {
		if (list->firstId == 0) {
			ID newItemId = 0;
			ListItem* newItem = Bucket_Mark(&list->bucket, NULL, &newItemId);
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
			ID oldFirstItemId = list->firstId;
			ListItem* oldFirstItem = Bucket_GetById(&list->bucket, oldFirstItemId);
			ID newFirstItemId = 0;
			ListItem* newFirstItem = Bucket_Mark(&list->bucket, NULL, &newFirstItemId);
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
		ListItem* iteratorItem = Bucket_GetById(&list->bucket, afterIterator);
		if (!iteratorItem) {
			return NULL;
		}
		if (iteratorItem->nextId == 0) {
			ID newLastItemId = 0;
			ListItem* newLastItem = Bucket_Mark(&list->bucket, NULL, &newLastItemId);
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
			ID nextItemId = iteratorItem->nextId;
			ListItem* nextItem = Bucket_GetById(&list->bucket, nextItemId);
			ID newItemId = 0;
			ListItem* newItem = Bucket_Mark(&list->bucket, NULL, &newItemId);
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

void* ListInsertBefore(List* list, ID beforeIterator, void* copy, ID* outIterator) {
	if (beforeIterator == 0) {
		return ListInsertAfter(list, list->lastId, copy, outIterator);
	} else {
		ListItem* iteratorItem = Bucket_GetById(&list->bucket, beforeIterator);
		if (!iteratorItem) {
			return NULL;
		}
		return ListInsertAfter(list, iteratorItem->prevId, copy, outIterator);
	}
}

void ListRemove(List* list, ID iterator) {
	ListItem* item = Bucket_GetById(&list->bucket, iterator);
	if (!item) {
		return;
	}
	
	if (iterator == list->firstId && iterator == list->lastId) {
		list->firstId = 0;
		list->lastId = 0;
	} else if (iterator == list->firstId) {
		list->firstId = item->nextId;
		((ListItem*)Bucket_GetById(&list->bucket, item->nextId))->prevId = 0;
	} else if (iterator == list->lastId) {
		list->lastId = item->prevId;
		((ListItem*)Bucket_GetById(&list->bucket, item->prevId))->nextId = 0;
	} else {
		((ListItem*)Bucket_GetById(&list->bucket, item->nextId))->prevId = item->prevId;
		((ListItem*)Bucket_GetById(&list->bucket, item->prevId))->nextId = item->nextId;
	}
	Bucket_UnmarkById(&list->bucket, iterator);
}

void ListClear(List* list) {
	Bucket_UnmarkAll(&list->bucket);
	list->firstId = 0;
	list->lastId = 0;
}

size_t ListLength(List* list) {
	return list->bucket.size;
}

ID ListGetFirst(List* list) {
	return list->firstId;
}

ID ListGetByIndex(List* list, size_t index) {
	ID iterator = list->firstId;
	for (size_t i = 0; i < index; i++) {
		ListItem* item = Bucket_GetById(&list->bucket, iterator);
		if (item) {
			iterator = item->nextId;
		} else {
			return 0;
		}
	}
	return iterator;
}

ID ListGetNext(List* list, ID iterator) {
	ListItem* item = Bucket_GetById(&list->bucket, iterator);
	return item ? item->nextId : 0;
}

ID ListGetPrev(List* list, ID iterator) {
	ListItem* item = Bucket_GetById(&list->bucket, iterator);
	return item ? item->prevId : 0;
}

ID ListGetLast(List* list) {
	return list->lastId;
}

void* ListGetData(List* list, ID iterator) {
	ListItem* item = Bucket_GetById(&list->bucket, iterator);
	return item ? item->data : NULL;
}
