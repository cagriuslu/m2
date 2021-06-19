#ifndef BUCKET_H
#define BUCKET_H

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
	int capacity; // 65536
	int size; // [0, 65536]
	int nextKey; // [1, 65536]
	int highestAllocatedIndex;
	int lowestAllocatedIndex;
	int nextFreeIndex;
} Bucket;

int BucketInit(Bucket* bucket, size_t dataSize);
void BucketDeinit(Bucket* bucket);

void* BucketMark(Bucket* bucket, void* copy, uint64_t* outId);
void BucketUnmark(Bucket* bucket, void* data);
void BucketUnmarkByIndex(Bucket* bucket, int idx);
void BucketUnmarkById(Bucket* bucket, uint64_t id);
void BucketUnmarkAll(Bucket* bucket);

bool BucketIsMarked(Bucket* bucket, void* data);
bool BucketIsMarkedByIndex(Bucket* bucket, int idx);
bool BucketIsMarkedById(Bucket* bucket, uint64_t id);
void* BucketGetFirst(Bucket* bucket);
void* BucketGetLast(Bucket* bucket);
void* BucketGetNext(Bucket* bucket, void* currData);
void* BucketGetPrev(Bucket* bucket, void* data);
void* BucketGetByIndex(Bucket* bucket, int idx);
void* BucketGetById(Bucket* bucket, uint64_t id);
uint64_t BucketGetId(Bucket* bucket, void* data);

#endif
