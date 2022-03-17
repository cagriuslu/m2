#include "m2/TinyArray.hh"
#include "m2/Pool.hh"
#include "m2/Def.hh"

uint64_t TinyArray_Heap[TINY_ARRAY_MAX_INSTANCE_COUNT * TINY_ARRAY_MAX_LENGTH];
Pool TinyArray_UsedArrays;

M2Err TinyArray_Init(TinyArray* array) {
	memset(array, 0, sizeof(TinyArray));

	// Init TinyArray_UsedArrays lazily
	if (TinyArray_UsedArrays.items == NULL) {
		M2ERR_REFLECT(Pool_Init(&TinyArray_UsedArrays, TINY_ARRAY_MAX_INSTANCE_COUNT_BITS, sizeof(uint64_t*)));
	}

	ID poolItemId;
	uint64_t** poolItemDataPtr = static_cast<uint64_t **>(Pool_Mark(&TinyArray_UsedArrays, NULL, &poolItemId));
	if (poolItemDataPtr) {
		array->tinyArrayId = Pool_GetIndex(&TinyArray_UsedArrays, poolItemId);
		*poolItemDataPtr = TinyArray_Heap + (TINY_ARRAY_MAX_LENGTH * sizeof(uint64_t)) * array->tinyArrayId;
		return M2OK;
	} else {
		return LOG_WARNING_M2(M2ERR_TINYOBJ_LIMIT_EXCEEDED);
	}
}

M2Err TinyArray_Append(TinyArray* array, uint64_t item) {
	if (array->length < TINY_ARRAY_MAX_LENGTH) {
		uint64_t** poolItemDataPtr = static_cast<uint64_t **>(Pool_GetByIndex(&TinyArray_UsedArrays,
																			  array->tinyArrayId));
		if (poolItemDataPtr) {
			(*poolItemDataPtr)[array->length] = item;
			array->length++;
			return M2OK;
		} else {
			return LOG_ERROR_M2V(M2ERR_IMPLEMENTATION, String, "TinyArray is used without being allocated");
		}
	} else {
		return M2ERR_LIMIT_EXCEEDED;
	}
}

M2Err TinyArray_Set(TinyArray* array, uint16_t index, uint64_t item) {
	if (index < array->length) {
		uint64_t** poolItemDataPtr = static_cast<uint64_t **>(Pool_GetByIndex(&TinyArray_UsedArrays,
																			  array->tinyArrayId));
		if (poolItemDataPtr) {
			(*poolItemDataPtr)[index] = item;
			return M2OK;
		} else {
			return LOG_ERROR_M2V(M2ERR_IMPLEMENTATION, String, "TinyArray is used without being allocated");
		}
	} else {
		return M2ERR_OUT_OF_BOUNDS;
	}
}

M2Err TinyArray_Remove(TinyArray* array, uint16_t index) {
	if (index < array->length) {
		uint64_t** poolItemDataPtr = static_cast<uint64_t **>(Pool_GetByIndex(&TinyArray_UsedArrays,
																			  array->tinyArrayId));
		if (poolItemDataPtr) {
			uint64_t* arrayPtr = *poolItemDataPtr;

			size_t nextIndex = index + 1;
			for (; nextIndex < array->length; nextIndex++) {
				arrayPtr[nextIndex - 1] = arrayPtr[nextIndex];
			}
			arrayPtr[nextIndex - 1] = 0;
			array->length--;
			return M2OK;
		} else {
			return LOG_ERROR_M2V(M2ERR_IMPLEMENTATION, String, "TinyArray is used without being allocated");
		}
	} else {
		return M2ERR_OUT_OF_BOUNDS;
	}
}

void TinyArray_Clear(TinyArray* array) {
	array->length = 0;
}

uint16_t TinyArray_Length(TinyArray* array) {
	return array->length;
}

uint64_t* TinyArray_Get(TinyArray* array, uint16_t index) {
	uint64_t** poolItemDataPtr = static_cast<uint64_t **>(Pool_GetByIndex(&TinyArray_UsedArrays, array->tinyArrayId));
	if (poolItemDataPtr && index < array->length) {
		return (*poolItemDataPtr) + index;
	} else {
		return NULL;
	}
}

void TinyArray_Term(TinyArray* array) {
	Pool_UnmarkByIndex(&TinyArray_UsedArrays, array->tinyArrayId);
	memset(array, 0, sizeof(TinyArray));
}
