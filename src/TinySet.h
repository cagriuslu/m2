#ifndef TINYSET_H
#define TINYSET_H

#include "TinyArray.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define TINY_SET_BUCKET_COUNT (4)

typedef struct _TinySet {
	TinyArray buckets[TINY_SET_BUCKET_COUNT];
} TinySet;

// Specializations
typedef TinySet TinySetOfIDs;

// Constructors
XErr TinySet_Init(TinySet* ts);

// Modifiers
XErr TinySet_Set(TinySet* ts, uint64_t key);
void TinySet_Unset(TinySet* ts, uint64_t key);
void TinySet_Clear(TinySet* ts);

// Accessors
bool TinySet_Check(TinySet* ts, uint64_t key);
size_t TinySet_Size(TinySet* ts);

// Destructor
void TinySet_Term(TinySet* ts);

#endif
