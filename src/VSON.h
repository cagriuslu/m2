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

VSON* VSON_Get(VSON* vson, const char* path);
VSON* VSON_GetObject(VSON* vson, const char* path);
VSON* VSON_GetArray(VSON* vson, const char* path);
const char* VSON_GetString(VSON* vson, const char* path);
long VSON_GetLong(VSON* vson, const char* path, long defaultValue);
float VSON_GetFloat(VSON* vson, const char* path, float defaultValue);

void VSON_Term(VSON* vson);

// Convenience macros
#define VSON_OBJECT_ITERATE(vson, objectKeyValuePtrName) for(VSONObjectKeyValue* objectKeyValuePtrName = (vson) ? (vson)->value.objectFirstChild : NULL; objectKeyValuePtrName && (vson)->type == VSON_VALUE_TYPE_OBJECT; objectKeyValuePtrName = objectKeyValuePtrName->next)

#endif
