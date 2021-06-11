#ifndef DRAW_LIST_H
#define DRAW_LIST_H

#include "GameObject.h"
#include "Array.h"

#define DrawObject(object) (DrawListInsert(CurrentDrawList(), object))

typedef struct _DrawList {
	Array objects;
} DrawList;

int DrawListInit(DrawList *list);
size_t DrawListLength(DrawList *list);
ObjectPtr DrawListGet(DrawList *list, size_t i);
ObjectPtr DrawListInsert(DrawList *list, ObjectPtr objptr);
void DrawListSort(DrawList *list);
void DrawListDeinit(DrawList *list);

#endif
