#include "Array.h"
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#define GROWTH_RATE (2.0)

XErr Array_Init(Array* array, size_t itemSize, size_t initCapacity, size_t maxSize, void (*itemTerm)(void*)) {
	memset(array, 0, sizeof(Array));
	assert(initCapacity <= maxSize);
	array->data = calloc(initCapacity, itemSize);
	assert(array->data);
	array->itemSize = itemSize;
	array->length = 0;
	array->capacity = initCapacity;
	array->maxSize = maxSize;
	array->itemTerm = itemTerm;
	return 0;
}

void* Array_Append(Array *array, void *item) {
	if (array->length < array->maxSize) {
		if (array->length < array->capacity) {
			if (item) {
				memcpy(array->data + array->length * array->itemSize, item, array->itemSize);
			}
			array->length++;
			return Array_GetLast(array);
		} else {
			size_t newCapacity = (size_t)round(array->capacity * GROWTH_RATE);
			void* newData = realloc(array->data, newCapacity * array->itemSize);
			assert(newData);
			array->data = newData;
			array->capacity = newCapacity;
			return Array_Append(array, item);
		}
	} else {
		return NULL;
	}
}

void Array_Remove(Array* array, size_t index) {
	if (index < array->length) {
		size_t nextIndex = index + 1;
		for (; nextIndex < array->length; nextIndex++) {
			memcpy(Array_Get(array, nextIndex - 1), Array_Get(array, nextIndex), array->itemSize);
		}
		memset(Array_Get(array, nextIndex - 1), 0, array->itemSize);
		array->length--;
	}
}

void Array_Clear(Array* array) {
	memset(array->data, 0, array->length * array->itemSize);
	array->length = 0;
}

XErr Array_Shrink(Array* array) {
	if (array->length < array->capacity) {
		size_t newCapacity = array->length;
		void* newData = realloc(array->data, newCapacity * array->itemSize);
		if (!newData) {
			return XERR_OUT_OF_MEMORY;
		}
		array->data = newData;
		array->capacity = newCapacity;
	}
	return 0;
}

size_t Array_Length(Array* array) {
	return array->length;
}

void* Array_Get(Array *array, size_t index) {
	if (index < array->length) {
		return array->data + index * array->itemSize;
	} else {
		return NULL;
	}
}

size_t Array_GetIndexOf(Array* array, void* item) {
	intptr_t itemOffset = ((char*) item) - array->data;
	intptr_t itemIndex = itemOffset / array->itemSize;
	if ((uintptr_t) itemIndex < array->length) {
		return itemIndex;
	} else {
		return (size_t)-1;
	}
}

void* Array_GetLast(Array *array) {
	if (array->length) {
		return Array_Get(array, array->length - 1);
	} else {
		return NULL;
	}
}

void Array_Term(Array *array) {
	if (array->itemTerm) {
		for (size_t i = 0; i < array->length; i++) {
			void* item = Array_Get(array, i);
			array->itemTerm(item);
		}
	}
	free(array->data);
	memset(array, 0, sizeof(Array));
}

void* Array_TermNoFree(Array* array) {
	return array->data;
}
