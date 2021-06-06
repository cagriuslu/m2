#include "Array.h"
#include <math.h>
#include <string.h>
#include <assert.h>

#define INITIAL_CAPACITY (16)
#define GROWTH_RATE (2.0)

int ArrayInit(Array *array, size_t itemSize) {
	array->data = malloc(itemSize * INITIAL_CAPACITY);
	assert(array->data);
	array->itemSize = itemSize;
	array->length = 0;
	array->capacity = INITIAL_CAPACITY;
	return 0;
}

size_t ArrayLength(Array *array) {
	return array->length;
}

void* ArrayAppend(Array *array, void *item) {
	if (array->length < array->capacity) {
		if (item) {
			memcpy(array->data + array->length * array->itemSize, item, array->itemSize);
		}
		array->length++;
		return ArrayGetLast(array);
	} else {
		size_t newCapacity = (size_t) round(array->capacity * GROWTH_RATE);
		void *newData = realloc(array->data, newCapacity * array->itemSize);
		assert(newData);
		array->data = newData;
		array->capacity = newCapacity;
		return ArrayAppend(array, item);
	}
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
