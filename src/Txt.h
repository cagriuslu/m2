#ifndef TXT_H
#define TXT_H

#include "HashMap.h"
#include "Array.h"
#include "Error.h"
#include <stdbool.h>

typedef struct _TxtKV {
	char key[8]; // 7 chars + nil
	const char* value;
	bool isValueDynamic;
} TxtKV;

XErr TxtKV_Init(TxtKV* kv);
void TxtKV_SetKey(TxtKV* kv, const char* key);
void TxtKV_SetValue(TxtKV* kv, const char* value, bool isDynamic);
char* TxtKV_DuplicateUrlEncodedValue(TxtKV* kv, const char* key); // example key: "Tile=", "Enemy="
void TxtKV_Term(TxtKV* kv);

typedef struct _Txt {
	ArrayOfTxtKVs txtKVPairs; // TODO turn this into HashMap
	HashMapOfInt32s txtKVIndexes;
} Txt;

XErr Txt_Init(Txt* txt);
XErr Txt_InitFromFile(Txt* txt, const char* fpath);
XErr Txt_SaveToFile(Txt* txt, const char* fpath);
void Txt_Term(Txt* txt);

#endif
