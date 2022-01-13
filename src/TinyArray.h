#ifndef TINYARRAY_H
#define TINYARRAY_H

#include "Error.h"
#include <stdint.h>

#define TINY_ARRAY_MAX_INSTANCE_COUNT_BITS (12)
#define TINY_ARRAY_MAX_INSTANCE_COUNT (1 << (TINY_ARRAY_MAX_INSTANCE_COUNT_BITS - 1)) // 4096
#define TINY_ARRAY_MAX_LENGTH (512)

typedef struct _TinyArray {
	uint16_t tinyArrayId;
	uint16_t length;
} TinyArray;

// Specializations
typedef TinyArray TinyArrayOfIDs;

// Constructors
XErr TinyArray_Init(TinyArray* array);

// Modifiers
XErr TinyArray_Append(TinyArray* array, uint64_t item);
XErr TinyArray_Set(TinyArray* array, uint16_t index, uint64_t item);
XErr TinyArray_Remove(TinyArray* array, uint16_t index);
void TinyArray_Clear(TinyArray* array);

// Accessors
uint16_t TinyArray_Length(TinyArray* array);
uint64_t* TinyArray_Get(TinyArray* array, uint16_t index);

// Destructor
void TinyArray_Term(TinyArray* array);

#endif
