#ifndef ARRAY_H
#define ARRAY_H

#include "Error.h"
#include <stdlib.h>

typedef struct _Array {
	char *data;
	size_t itemSize;
	size_t length;
	size_t capacity;
} Array;

int ArrayInit(Array *array, size_t itemSize);
size_t ArrayLength(Array *array);
void* ArrayAppend(Array *array, void *item);
void* ArrayGet(Array *array, size_t index);
size_t ArrayGetIndexOf(Array* array, void* item);
void* ArrayGetLast(Array *array);
void ArrayDeinit(Array *array);

#endif
