#ifndef ARRAY_H
#define ARRAY_H

#include <m2/Def.hh>
#include <stdlib.h>

typedef struct _Array {
	char *data;
	size_t itemSize;
	size_t length;
	size_t capacity;
	size_t maxLength;
	void (*itemTerm)(void*);
} Array;

// Specializations
typedef Array ArrayOfTxtKVs;
typedef Array ArrayOfVec2Is;
typedef Array ArrayOfVec2Fs;

// Constructors
M2Err Array_Init(Array* array, size_t itemSize, size_t initCapacity, size_t maxSize, void (*itemTerm)(void*));

// Modifiers
void* Array_Append(Array* array, void* item);
void Array_Remove(Array* array, size_t index);
void Array_Clear(Array* array);
M2Err Array_Shrink(Array* array);

// Accessors
size_t Array_Length(Array* array);
void* Array_Get(Array* array, size_t index);
size_t Array_GetIndexOf(Array* array, void* item);
void* Array_GetLast(Array* array);

// Destructor
void Array_Term(Array* array);
void* Array_TermNoFree(Array* array);

#define TO_ARRAY_ITEM_TERM(funcptr) ((void (*)(void*))(funcptr))

#endif