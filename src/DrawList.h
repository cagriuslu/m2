#ifndef DRAW_LIST_H
#define DRAW_LIST_H

#include "Object.h"
#include "Array.h"

typedef struct _DrawList {
	Array objects;
} DrawList;

int DrawListInit(DrawList *list);
size_t DrawListLength(DrawList *list);
ObjectPtr DrawListGet(DrawList *list, size_t i);
int DrawListInsert(DrawList *list, ObjectPtr objptr);
void DrawListSort(DrawList *list);
void DrawListDeinit(DrawList *list);

#endif
