#include "List.h"
#include "Error.h"
#include <string.h>

typedef struct _ListItem {
	ID nextId;
	ID prevId;
	char data[0];
} ListItem;

int List_Init(List* list, size_t itemSize) {
	memset(list, 0, sizeof(List));
	PROPAGATE_ERROR(Pool_Init(&list->bucket, 16, sizeof(ListItem) + itemSize));
	list->dataSize = itemSize;
	return 0;
}

void List_Term(List* list) {
	Pool_Term(&list->bucket);
	memset(list, 0, sizeof(List));
}

void* List_Append(List* list, void* copy, ID* outIterator) {
	return List_InsertAfter(list, list->lastId, copy, outIterator);
}

void* List_Prepend(List* list, void* copy, ID* outIterator) {
	return List_InsertAfter(list, 0, copy, outIterator);
}

void* List_InsertAfter(List* list, ID afterIterator, void* copy, ID* outIterator) {
	if (afterIterator == 0) {
		if (list->firstId == 0) {
			ID newItemId = 0;
			ListItem* newItem = Pool_Mark(&list->bucket, NULL, &newItemId);
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
			ListItem* oldFirstItem = Pool_GetById(&list->bucket, oldFirstItemId);
			ID newFirstItemId = 0;
			ListItem* newFirstItem = Pool_Mark(&list->bucket, NULL, &newFirstItemId);
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
		ListItem* iteratorItem = Pool_GetById(&list->bucket, afterIterator);
		if (!iteratorItem) {
			return NULL;
		}
		if (iteratorItem->nextId == 0) {
			ID newLastItemId = 0;
			ListItem* newLastItem = Pool_Mark(&list->bucket, NULL, &newLastItemId);
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
			ListItem* nextItem = Pool_GetById(&list->bucket, nextItemId);
			ID newItemId = 0;
			ListItem* newItem = Pool_Mark(&list->bucket, NULL, &newItemId);
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

void* List_InsertBefore(List* list, ID beforeIterator, void* copy, ID* outIterator) {
	if (beforeIterator == 0) {
		return List_InsertAfter(list, list->lastId, copy, outIterator);
	} else {
		ListItem* iteratorItem = Pool_GetById(&list->bucket, beforeIterator);
		if (!iteratorItem) {
			return NULL;
		}
		return List_InsertAfter(list, iteratorItem->prevId, copy, outIterator);
	}
}

void List_Remove(List* list, ID iterator) {
	ListItem* item = Pool_GetById(&list->bucket, iterator);
	if (!item) {
		return;
	}
	
	if (iterator == list->firstId && iterator == list->lastId) {
		list->firstId = 0;
		list->lastId = 0;
	} else if (iterator == list->firstId) {
		list->firstId = item->nextId;
		((ListItem*)Pool_GetById(&list->bucket, item->nextId))->prevId = 0;
	} else if (iterator == list->lastId) {
		list->lastId = item->prevId;
		((ListItem*)Pool_GetById(&list->bucket, item->prevId))->nextId = 0;
	} else {
		((ListItem*)Pool_GetById(&list->bucket, item->nextId))->prevId = item->prevId;
		((ListItem*)Pool_GetById(&list->bucket, item->prevId))->nextId = item->nextId;
	}
	Pool_UnmarkById(&list->bucket, iterator);
}

void List_Clear(List* list) {
	Pool_UnmarkAll(&list->bucket);
	list->firstId = 0;
	list->lastId = 0;
}

size_t List_Length(List* list) {
	return list->bucket.size;
}

ID List_GetFirst(List* list) {
	return list->firstId;
}

ID List_GetByIndex(List* list, size_t index) {
	ID iterator = list->firstId;
	for (size_t i = 0; i < index; i++) {
		ListItem* item = Pool_GetById(&list->bucket, iterator);
		if (item) {
			iterator = item->nextId;
		} else {
			return 0;
		}
	}
	return iterator;
}

ID List_GetNext(List* list, ID iterator) {
	ListItem* item = Pool_GetById(&list->bucket, iterator);
	return item ? item->nextId : 0;
}

ID List_GetPrev(List* list, ID iterator) {
	ListItem* item = Pool_GetById(&list->bucket, iterator);
	return item ? item->prevId : 0;
}

ID List_GetLast(List* list) {
	return list->lastId;
}

void* List_GetData(List* list, ID iterator) {
	ListItem* item = Pool_GetById(&list->bucket, iterator);
	return item ? item->data : NULL;
}
