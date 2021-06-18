#include "Array.h"
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#define GROWTH_RATE (2.0)

int ArrayInit(Array* array, size_t itemSize, size_t initCapacity, size_t maxSize) {
	assert(initCapacity <= maxSize);
	array->data = calloc(initCapacity, itemSize);
	assert(array->data);
	array->itemSize = itemSize;
	array->length = 0;
	array->capacity = initCapacity;
	array->maxSize = maxSize;
	return 0;
}

void* ArrayAppend(Array *array, void *item) {
	if (array->length < array->maxSize) {
		if (array->length < array->capacity) {
			if (item) {
				memcpy(array->data + array->length * array->itemSize, item, array->itemSize);
			}
			array->length++;
			return ArrayGetLast(array);
		} else {
			size_t newCapacity = (size_t)round(array->capacity * GROWTH_RATE);
			void* newData = realloc(array->data, newCapacity * array->itemSize);
			assert(newData);
			array->data = newData;
			array->capacity = newCapacity;
			return ArrayAppend(array, item);
		}
	} else {
		return NULL;
	}
}

void ArrayRemove(Array* array, size_t index) {
	if (index < array->length) {
		size_t nextIndex = index + 1;
		for (; nextIndex < array->length; nextIndex++) {
			memcpy(ArrayGet(array, nextIndex - 1), ArrayGet(array, nextIndex), array->itemSize);
		}
		memset(ArrayGet(array, nextIndex - 1), 0, array->itemSize);
		array->length--;
	}
}

void ArrayClear(Array* array) {
	memset(array->data, 0, array->length * array->itemSize);
	array->length = 0;
}

void* ArrayGet(Array *array, size_t index) {
	if (index < array->length) {
		return array->data + index * array->itemSize;
	} else {
		return NULL;
	}
}

size_t ArrayGetIndexOf(Array* array, void* item) {
	intptr_t itemOffset = ((char*) item) - array->data;
	intptr_t itemIndex = itemOffset / array->itemSize;
	if ((uintptr_t) itemIndex < array->length) {
		return itemIndex;
	} else {
		return (size_t)-1;
	}
}

void* ArrayGetLast(Array *array) {
	if (array->length) {
		return ArrayGet(array, array->length - 1);
	} else {
		return NULL;
	}
}

void ArrayDeinit(Array *array) {
	free(array->data);
}
