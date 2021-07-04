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
typedef Array ArrayOfTxtKVs;

XErr Array_Init(Array *array, size_t itemSize, size_t initCapacity, size_t maxSize);

void* Array_Append(Array* array, void* item);
void Array_Remove(Array* array, size_t index);
void Array_Clear(Array* array);

size_t Array_Length(Array* array);
void* Array_Get(Array* array, size_t index);
size_t Array_GetIndexOf(Array* array, void* item);
void* Array_GetLast(Array* array);

void Array_Term(Array *array);

#endif
