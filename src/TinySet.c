#include "TinySet.h"
#include <string.h>

static uint8_t TinySet_Hash(const uint8_t* key) {
	uint8_t hash = 0x03;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 4; j++) {
			hash ^= key[i] >> (j*2);
		}
	}
	return hash & 0x3;
}

M2Err TinySet_Init(TinySet* ts) {
	memset(ts, 0, sizeof(TinySet));
	for (int i = 0; i < TINY_SET_BUCKET_COUNT; i++) {
		M2ERR_REFLECT(TinyArray_Init(ts->buckets + i));
	}
	return M2OK;
}

M2Err TinySet_Set(TinySet* ts, uint64_t key) {
	if (TinySet_Check(ts, key)) {
		return M2OK;
	} else {
		TinyArray* ta = ts->buckets + TinySet_Hash((uint8_t*) &key);
		return TinyArray_Append(ta, key);
	}
}

void TinySet_Unset(TinySet* ts, uint64_t key) {
	TinyArray* ta = ts->buckets + TinySet_Hash((uint8_t*) &key);
	for (int i = 0; i < TinyArray_Length(ta); i++) {
		uint64_t* item = TinyArray_Get(ta, i);
		if (item && *item == key) {
			TinyArray_Remove(ta, i);
			break;
		}
	}
}

void TinySet_Clear(TinySet* ts) {
	for (int i = 0; i < TINY_SET_BUCKET_COUNT; i++) {
		TinyArray_Clear(ts->buckets + i);
	}
}

bool TinySet_Check(TinySet* ts, uint64_t key) {
	TinyArray* ta = ts->buckets + TinySet_Hash((uint8_t*) &key);

	bool found = false;
	for (int i = 0; i < TinyArray_Length(ta); i++) {
		uint64_t* item = TinyArray_Get(ta, i);
		if (item && *item == key) {
			found = true;
			break;
		}
	}
	return found;
}

size_t TinySet_Size(TinySet* ts) {
	size_t count = 0;
	for (int i = 0; i < TINY_SET_BUCKET_COUNT; i++) {
		count += TinyArray_Length(ts->buckets + i);
	}
	return count;
}

void TinySet_Term(TinySet* ts) {
	for (int i = 0; i < TINY_SET_BUCKET_COUNT; i++) {
		TinyArray_Term(ts->buckets + i);
	}
	memset(ts, 0, sizeof(TinySet));
}
