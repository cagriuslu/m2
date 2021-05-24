#ifndef OBJECT_DRAW_LIST_H
#define OBJECT_DRAW_LIST_H

#include "Object.h"
#include "Array.h"

typedef struct _ObjectDrawList {
	Array objects;
} ObjectDrawList;

int ObjectDrawListInit(ObjectDrawList *list);
size_t ObjectDrawListLength(ObjectDrawList *list);
ObjectPtr ObjectDrawListGet(ObjectDrawList *list, size_t i);
int ObjectDrawListInsert(ObjectDrawList *list, ObjectPtr objptr);
void ObjectDrawListSort(ObjectDrawList *list);
void ObjectDrawListDeinit(ObjectDrawList *list);

#endif
