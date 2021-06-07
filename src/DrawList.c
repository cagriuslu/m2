#include "DrawList.h"
#include "Error.h"

#define AsObjectPtr(objptr) ((ObjectPtr*) (objptr))

int DrawListInit(DrawList *list) {
	PROPAGATE_ERROR(ArrayInit(&list->objects, sizeof(ObjectPtr), UINT16_MAX + 1, UINT16_MAX + 1));
	return 0;
}

size_t DrawListLength(DrawList *list) {
	return list->objects.length;
}

ObjectPtr DrawListGet(DrawList *list, size_t i) {
	return *AsObjectPtr(ArrayGet(&list->objects, i));
}

ObjectPtr DrawListInsert(DrawList *list, ObjectPtr objptr) {
	ArrayAppend(&list->objects, &objptr);
	// TODO this can be optimized with binary search
	// ONLY if we can make sure that the list is already sorted
	return objptr;
}

void DrawListSort(DrawList *list) {
	size_t len = DrawListLength(list);
	if (1 < len) {
		for (size_t i = 1; i < len; i++) {
			ObjectPtr currObjPtr = DrawListGet(list, i);
			float currY = currObjPtr->pos.y;

			for (size_t j = i; 0 < j--; ) {
				ObjectPtr iterObjPtr = DrawListGet(list, j);
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

void DrawListDeinit(DrawList *list) {
	ArrayDeinit(&list->objects);
}
