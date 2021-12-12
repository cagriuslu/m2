#define _CRT_SECURE_NO_WARNINGS
#include "VSON.h"
#include "Array.h"
#include "String.h"
#include "Defs.h"
#include "Log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#define ISPLAIN(c) (isalnum(c) || (c) == '_' || (c) == '-' || (c) == '.')

XErr _VSON_ParseFile_UnknownValue(VSON* vson, FILE* f);
void _VSON_Serialize_ToFile_AnyValue(VSON* vson, FILE* file);

XErr VSON_Init_ParseFile(VSON* vson, const char* path) {
	memset(vson, 0, sizeof(VSON));
	FILE* f = fopen(path, "r");
	if (!f) {
		return XERR_FILE_NOT_FOUND;
	}

	XErr result = _VSON_ParseFile_UnknownValue(vson, f);
	if (result == 0 && !feof(f)) {
		// Check if anything left in file
		int c;
		while ((c = fgetc(f)) != EOF) {
			if (!isspace(c)) {
				result = XERR_FILE_CORRUPTED;
				break;
			}
		}
	}

	fclose(f);
	return result;
}

XErr VSON_InitObject(VSON* vson) {
	memset(vson, 0, sizeof(VSON));
	vson->type = VSON_VALUE_TYPE_OBJECT;
	return 0;
}

XErr VSON_InitArray(VSON* vson) {
	memset(vson, 0, sizeof(VSON));
	vson->type = VSON_VALUE_TYPE_ARRAY;
	return 0;
}

XErr VSON_InitString(VSON* vson, const char* string) {
	memset(vson, 0, sizeof(VSON));
	vson->type = VSON_VALUE_TYPE_STRING;
	if ((vson->value.string = string ? STRDUP(string) : calloc(1, 1)) == NULL) {
		return XERR_OUT_OF_MEMORY;
	}
	return 0;
}

XErr VSON_InitString_NoCopy(VSON* vson, const char* string) {
	memset(vson, 0, sizeof(VSON));
	vson->type = VSON_VALUE_TYPE_STRING;
	if ((vson->value.string = string ? string : calloc(1, 1)) == NULL) {
		return XERR_OUT_OF_MEMORY;
	}
	return 0;
}

XErr _VSON_ParseFile_FetchPlainString(Array* buffer, FILE* f) {
	XErr result = Array_Init(buffer, sizeof(char), 16, INT32_MAX, NULL);
	if (result) {
		return result;
	}

	int c;
	char ch;
	while ((c = fgetc(f)) != EOF) {
		if (ISPLAIN(c)) {
			ch = (char)c;
			if (!Array_Append(buffer, &ch)) {
				// out of memory, or max size reached
				Array_Term(buffer);
				return XERR_FILE_CORRUPTED;
			}
		} else {
			// plain string finished
			ungetc(c, f);
			break;
		}
	}

	ch = 0;
	if (!Array_Append(buffer, &ch)) {
		// out of memory, or max size reached
		Array_Term(buffer);
		return XERR_FILE_CORRUPTED;
	}
	
	return 0;
}

XErr _VSON_ParseFile_ObjectValue(VSON* vson, FILE* f) {
	XErr result = VSON_InitObject(vson);
	if (result) {
		return result;
	}
	VSONObjectKeyValue** nextObjectKeyValuePointerLocation = &(vson->value.objectFirstChild);
	VSONObjectKeyValue* currentObjectKeyValue = NULL;

	const int EXPECT_KEY = 0;
	const int EXPECT_COLON = 1;
	const int EXPECT_VALUE = 2;
	const int EXPECT_COMMA_OR_SPACE = 3;

	int c, braceClosed = 0, state = EXPECT_KEY;
	while ((c = fgetc(f)) != EOF) {
		if (state == EXPECT_KEY) {
			if (c == '}') {
				braceClosed = 1;
				break;
			} else if (isspace(c)) {
				// Do nothing
			} else if (ISPLAIN(c)) {
				// Create key-value pair
				currentObjectKeyValue = calloc(1, sizeof(VSONObjectKeyValue));
				if (!currentObjectKeyValue) {
					return XERR_OUT_OF_MEMORY;
				}
				nextObjectKeyValuePointerLocation[0] = currentObjectKeyValue;
				nextObjectKeyValuePointerLocation = &(currentObjectKeyValue->next);
				// Parse key
				ungetc(c, f);
				Array buffer;
				result = _VSON_ParseFile_FetchPlainString(&buffer, f);
				if (result) {
					return result;
				}
				currentObjectKeyValue->key = Array_TermNoFree(&buffer);
				// Next state
				state = EXPECT_COLON;
			} else {
				return XERR_FILE_CORRUPTED;
			}
		} else if (state == EXPECT_COLON) {
			if (isspace(c)) {
				// Do nothing
			} else if (c == ':') {
				// Next state
				state = EXPECT_VALUE;
			} else {
				return XERR_FILE_CORRUPTED;
			}
		} else if (state == EXPECT_VALUE) {
			if (isspace(c)) {
				// Do nothing
			} else {
				// Parse value
				ungetc(c, f);
				result = _VSON_ParseFile_UnknownValue(&currentObjectKeyValue->value, f);
				if (result) {
					return XERR_FILE_CORRUPTED;
				}
				// Next state
				state = EXPECT_COMMA_OR_SPACE;
			}
		} else {
			if (c == '}') {
				braceClosed = 1;
				break;
			} else if (c == ',' || isspace(c)) {
				// Next state
				state = EXPECT_KEY;
			} else {
				return XERR_FILE_CORRUPTED;
			}
		}
	}

	if (!braceClosed) {
		return XERR_FILE_CORRUPTED;
	}
	return 0;
}

XErr _VSON_ParseFile_ArrayValue(VSON* vson, FILE* f) {
	XErr result = VSON_InitArray(vson);
	if (result) {
		return result;
	}
	VSONArrayValue** nextArrayValuePointerLocation = &(vson->value.arrayFirstChild);

	const int EXPECT_VALUE = 0;
	const int EXPECT_COMMA_OR_SPACE = 1;
	
	int c, bracketClosed = 0, state = EXPECT_VALUE;
	while ((c = fgetc(f)) != EOF) {
		if (state == EXPECT_VALUE) {
			if (c == ']') {
				bracketClosed = 1;
				break;
			} else if (isspace(c)) {
				// Do nothing
			} else {
				VSONArrayValue* arrayValue = calloc(1, sizeof(VSONArrayValue));
				if (!arrayValue) {
					return XERR_OUT_OF_MEMORY;
				}

				nextArrayValuePointerLocation[0] = arrayValue;
				nextArrayValuePointerLocation = &(arrayValue->next);

				ungetc(c, f);
				result = _VSON_ParseFile_UnknownValue(&arrayValue->value, f);
				if (result) {
					return XERR_FILE_CORRUPTED;
				}
				
				state = EXPECT_COMMA_OR_SPACE;
			}
		} else {
			if (c == ']') {
				bracketClosed = 1;
				break;
			} else if (c == ',' || isspace(c)) {
				// Next state
				state = EXPECT_VALUE;
			} else {
				return XERR_FILE_CORRUPTED;
			}
		}
	}

	if (!bracketClosed) {
		return XERR_FILE_CORRUPTED;
	}
	return 0;
}

XErr _VSON_ParseFile_QuoteStringValue(VSON* vson, FILE* f) {
	Array buffer;
	XErr result = Array_Init(&buffer, sizeof(char), 32, INT32_MAX, NULL);
	if (result) {
		return result;
	}

	int c, escaping = 0, quoteClosed = 0;
	while ((c = fgetc(f)) != EOF) {
		if (escaping) {
			if (c == '"' || c == '\\') {
				if (Array_Append(&buffer, &c) == NULL) {
					// out of memory, or max size reached
					Array_Term(&buffer);
					return XERR_FILE_CORRUPTED;
				}
				escaping = 0;
			} else {
				// unexpected character
				Array_Term(&buffer);
				return XERR_FILE_CORRUPTED;
			}
		} else {
			if (c == '"') {
				// String finished
				quoteClosed = 1;
				break;
			} else if (c == '\\') {
				escaping = 1;
			} else if (isprint(c)) {
				if (Array_Append(&buffer, &c) == NULL) {
					// out of memory, or max size reached
					Array_Term(&buffer);
					return XERR_FILE_CORRUPTED;
				}
			} else {
				// unexpected character
				Array_Term(&buffer);
				return XERR_FILE_CORRUPTED;
			}
		}
	}

	if (!quoteClosed) {
		// Quote not closed
		Array_Term(&buffer);
		return XERR_FILE_CORRUPTED;
	}

	c = 0;
	if (Array_Append(&buffer, &c) == NULL) {
		// out of memory, or max size reached
		Array_Term(&buffer);
		return XERR_FILE_CORRUPTED;
	}

	result = VSON_InitString(vson, Array_Get(&buffer, 0));
	Array_Term(&buffer);
	return result;
}

XErr _VSON_ParseFile_PlainStringValue(VSON* vson, FILE* f) {
	Array buffer;
	XErr result = _VSON_ParseFile_FetchPlainString(&buffer, f);
	if (result) {
		return result;
	}
	return VSON_InitString_NoCopy(vson, Array_TermNoFree(&buffer));
}

XErr _VSON_ParseFile_UnknownValue(VSON* vson, FILE* f) {
	int c, result = 1;
	while ((c = fgetc(f)) != EOF) {
		if (isspace(c)) {
			// Do nothing
		} else {
			if (c == '{') {
				// Object
				result = _VSON_ParseFile_ObjectValue(vson, f);
			} else if (c == '[') {
				// Array
				result = _VSON_ParseFile_ArrayValue(vson, f);
			} else if (c == '"') {
				// Quoted string
				result = _VSON_ParseFile_QuoteStringValue(vson, f);
			} else if (ISPLAIN(c)) {
				// Plain string
				ungetc(c, f);
				result = _VSON_ParseFile_PlainStringValue(vson, f);
			} else {
				result = XERR_FILE_CORRUPTED;
			}
			break;
		}
	}
	if (result == 1) {
		// Empty file
		result = VSON_InitString(vson, NULL);
	}
	return result;
}

VSON* VSON_Get(VSON* vson, const char* path) {
	Array pathPieces;
	XErr result = String_Split(path, '/', &pathPieces);
	if (result) {
		return NULL;
	}

	for (size_t i = 0; i < Array_Length(&pathPieces) && vson; i++) {
		char** pathPiecePtr = Array_Get(&pathPieces, i);
		if (vson->type == VSON_VALUE_TYPE_OBJECT) {
			// Look for pathPiece
			VSONObjectKeyValue* objKV = vson->value.objectFirstChild;
			while (objKV) {
				if (strcmp(objKV->key, *pathPiecePtr) == 0) {
					break;
				}
				objKV = objKV->next;
			}
			vson = objKV ? &(objKV->value) : NULL;
		} else if (vson->type == VSON_VALUE_TYPE_ARRAY) {
			// Look for int(pathPiece)
			const long index = strtol(*pathPiecePtr, NULL, 10);
			VSONArrayValue* arrV = vson->value.arrayFirstChild;
			for (long ii = 0; ii < index && arrV; ii++) {
				arrV = arrV->next;
			}
			vson = arrV ? &(arrV->value) : NULL;
		} else {
			vson = NULL;
		}
	}

	Array_Term(&pathPieces);
	return vson;
}
const char* VSON_GetString(VSON* vson, const char* path) {
	VSON* strPtr = VSON_Get(vson, path);
	return (strPtr && strPtr->type == VSON_VALUE_TYPE_STRING) ? strPtr->value.string : NULL;
}
long VSON_GetLong(VSON* vson, const char* path, long defaultValue) {
	const char* str = VSON_GetString(vson, path);
	if (str) {
		char* str_end = NULL;
		long value = strtol(str, &str_end, 0);
		if (str != str_end) {
			return value;
		}
	}
	return defaultValue;
}
float VSON_GetFloat(VSON* vson, const char* path, float defaultValue) {
	const char* str = VSON_GetString(vson, path);
	if (str) {
		char* str_end = NULL;
		float value = strtof(str, &str_end);
		if (str != str_end) {
			return value;
		}
	}
	return defaultValue;
}

void _VSON_Serialize_ToFile_PrintString(FILE* file, const char* str) {
	fprintf(file, "\"");
	for (size_t i = 0; str[i] != 0; i++) {
		char c = str[i];
		if (c == '\\') {
			fprintf(file, "\\\\");
		} else if (c == '"') {
			fprintf(file, "\\\"");
		} else {
			fprintf(file, "%c", c);
		}
	}
	fprintf(file, "\"");
}

void _VSON_Serialize_ToFile_ObjectValue(VSON* vson, FILE* file) {
	fprintf(file, "{");
	VSON_OBJECT_ITERATE(vson, objKeyValuePtr) {
		_VSON_Serialize_ToFile_PrintString(file, objKeyValuePtr->key);
		fprintf(file, ":");
		_VSON_Serialize_ToFile_AnyValue(&objKeyValuePtr->value, file);
		fprintf(file, ",");
	}
	fprintf(file, "}");
}

void _VSON_Serialize_ToFile_ArrayValue(VSON* vson, FILE* file) {
	fprintf(file, "[");
	VSON_ARRAY_ITERATE(vson, arrayValuePtrName) {
		_VSON_Serialize_ToFile_AnyValue(&arrayValuePtrName->value, file);
		fprintf(file, ",");
	}
	fprintf(file, "]");
}

void _VSON_Serialize_ToFile_StringValue(VSON* vson, FILE* file) {
	_VSON_Serialize_ToFile_PrintString(file, vson->value.string);
}

void _VSON_Serialize_ToFile_AnyValue(VSON* vson, FILE* file) {
	switch (vson->type) {
		case VSON_VALUE_TYPE_OBJECT:
			return _VSON_Serialize_ToFile_ObjectValue(vson, file);
		case VSON_VALUE_TYPE_ARRAY:
			return _VSON_Serialize_ToFile_ArrayValue(vson, file);
		case VSON_VALUE_TYPE_STRING:
			return _VSON_Serialize_ToFile_StringValue(vson, file);
		default:
			break;
	}
}

XErr VSON_Serialize_ToFile(VSON* vson, const char* path) {
	FILE* file = fopen(path, "w");
	if (!file) {
		LOGXV_ERR(XERR_ERRNO, String, strerror(errno));
		LOGXV_ERR(XERR_FILE_INACCESSIBLE, String, path);
		return XERR_FILE_INACCESSIBLE;
	}

	_VSON_Serialize_ToFile_AnyValue(vson, file);
	fclose(file);
	return XOK;
}

void VSON_Term(VSON* vson) {
	(void)vson;
	// TODO
}
