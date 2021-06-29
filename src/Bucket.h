#ifndef BUCKET_H
#define BUCKET_H

#include "Error.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#pragma warning(disable : 4200)
typedef struct _BucketItem {
	uint32_t id; // If allocated: key|index, else: 0|nextFreeIndex
	char data[];
} BucketItem;

typedef struct _Bucket {
	BucketItem* items;
	size_t dataSize;
	size_t itemSize;
	uint64_t bucketId;
	size_t capacity; // 65536
	size_t size; // [0, 65536]
	size_t nextKey; // [1, 65536]
	size_t highestAllocatedIndex;
	size_t lowestAllocatedIndex;
	size_t nextFreeIndex;
} Bucket;

typedef uint64_t ID;

XErr Bucket_Init(Bucket* bucket, size_t dataSize);
void Bucket_Term(Bucket* bucket);

void* Bucket_Mark(Bucket* bucket, void* copy, ID* outId);
void Bucket_Unmark(Bucket* bucket, void* data);
void Bucket_UnmarkByIndex(Bucket* bucket, size_t idx);
void Bucket_UnmarkById(Bucket* bucket, ID id);
void Bucket_UnmarkAll(Bucket* bucket);

bool Bucket_IsMarked(Bucket* bucket, void* data);
bool Bucket_IsMarkedByIndex(Bucket* bucket, size_t idx);
bool Bucket_IsMarkedById(Bucket* bucket, ID id);
void* Bucket_GetFirst(Bucket* bucket);
void* Bucket_GetLast(Bucket* bucket);
void* Bucket_GetNext(Bucket* bucket, void* currData);
void* Bucket_GetPrev(Bucket* bucket, void* data);
void* Bucket_GetByIndex(Bucket* bucket, size_t idx);
void* Bucket_GetById(Bucket* bucket, ID id);
ID Bucket_GetId(Bucket* bucket, void* data);

#endif
