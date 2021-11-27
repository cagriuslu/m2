#ifndef VSON_H
#define VSON_H

#include "HashMap.h"
#include "Error.h"

typedef enum _VSONValueType {
	VSON_VALUE_TYPE_NIL = 0,
	VSON_VALUE_TYPE_OBJECT,
	VSON_VALUE_TYPE_ARRAY,
	VSON_VALUE_TYPE_STRING
} VSONValueType;

typedef union _VSONValue {
	struct _VSONObjectKeyValue* objectFirstChild;
	struct _VSONArrayValue* arrayFirstChild;
	const char* string;
} VSONValue;

typedef struct _VSON {
	VSONValueType type;
	VSONValue value;
} VSON;

typedef struct _VSONObjectKeyValue {
	const char* key;
	struct _VSON value;
	struct _VSONObjectKeyValue* next;
} VSONObjectKeyValue;

typedef struct _VSONArrayValue {
	struct _VSON value;
	struct _VSONArrayValue* next;
} VSONArrayValue;

XErr VSON_InitObject(VSON* vson);
XErr VSON_InitArray(VSON* vson);
XErr VSON_InitString(VSON* vson, const char* string);
XErr VSON_InitStringNoCopy(VSON* vson, const char* string);
XErr VSON_InitParseFile(VSON* vson, const char* path);

XErr VSON_Object_CreateHashMap(VSON* vson, HashMap* outHashMap);

VSON* VSON_Get(VSON* vson, const char* path);

void VSON_Term(VSON* vson);

#endif
