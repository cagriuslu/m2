#ifndef ARRAY_H
#define ARRAY_H

#include "Error.h"
#include <stdlib.h>

typedef struct _Array {
	char *data;
	size_t itemSize;
	size_t length;
	size_t capacity;
	size_t maxSize;
} Array;

int ArrayInit(Array *array, size_t itemSize, size_t initCapacity, size_t maxSize);
void* ArrayAppend(Array *array, void *item);
void ArrayRemove(Array* array, size_t index);
void ArrayClear(Array* array);
void* ArrayGet(Array *array, size_t index);
size_t ArrayGetIndexOf(Array* array, void* item);
void* ArrayGetLast(Array *array);
void ArrayDeinit(Array *array);

#endif
