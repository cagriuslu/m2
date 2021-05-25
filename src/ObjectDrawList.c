#include "ObjectDrawList.h"
#include "Error.h"

#define AsObjectPtr(objptr) ((ObjectPtr*) (objptr))

int ObjectDrawListInit(ObjectDrawList *list) {
	PROPAGATE_ERROR(ArrayInit(&list->objects, sizeof(ObjectPtr)));
	return 0;
}

size_t ObjectDrawListLength(ObjectDrawList *list) {
	return ArrayLength(&list->objects);
}

ObjectPtr ObjectDrawListGet(ObjectDrawList *list, size_t i) {
	return *AsObjectPtr(ArrayGet(&list->objects, i));
}

int ObjectDrawListInsert(ObjectDrawList *list, ObjectPtr objptr) {
	ArrayAppend(&list->objects, &objptr);
	ObjectDrawListSort(list);
	// TODO this can be optimized with binary search
	// ONLY if we can make sure that the list is already sorted
	return 0;
}

void ObjectDrawListSort(ObjectDrawList *list) {
	size_t len = ObjectDrawListLength(list);
	if (1 < len) {
		for (size_t i = 1; i < len; i++) {
			ObjectPtr currObjPtr = ObjectDrawListGet(list, i);
			float currY = currObjPtr->pos.y;

			for (size_t j = i; 0 < j--; ) {
				ObjectPtr iterObjPtr = ObjectDrawListGet(list, j);
				float iterY = iterObjPtr->pos.y;
				if (currY < iterY) {
					// Swap iter with its next item
					*AsObjectPtr(ArrayGet(&list->objects, j + 1)) = iterObjPtr;
					// Put curr object in iter's place
					*AsObjectPtr(ArrayGet(&list->objects, j)) = currObjPtr;
					// TODO this can be optimized
					// Don't copy the item in place every time
				} else {
					break;
				}
			}
		}
	}
}

void ObjectDrawListDeinit(ObjectDrawList *list) {
	ArrayDeinit(&list->objects);
}
