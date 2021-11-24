#define _CRT_SECURE_NO_WARNINGS
#include "VSON.h"
#include "Array.h"
#include "String.h"
#include "Defs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define ISPLAIN(c) (isalnum(c) || (c) == '_')

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

XErr VSON_InitStringNoCopy(VSON* vson, const char* string) {
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
				return XERR_CORRUPTED_FILE;
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
		return XERR_CORRUPTED_FILE;
	}
	
	return 0;
}

XErr _VSON_ParseFile_UnknownValue(VSON* vson, FILE* f);

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
				return XERR_CORRUPTED_FILE;
			}
		} else if (state == EXPECT_COLON) {
			if (isspace(c)) {
				// Do nothing
			} else if (c == ':') {
				// Next state
				state = EXPECT_VALUE;
			} else {
				return XERR_CORRUPTED_FILE;
			}
		} else if (state == EXPECT_VALUE) {
			if (isspace(c)) {
				// Do nothing
			} else {
				// Parse value
				ungetc(c, f);
				result = _VSON_ParseFile_UnknownValue(&currentObjectKeyValue->value, f);
				if (result) {
					return XERR_CORRUPTED_FILE;
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
				return XERR_CORRUPTED_FILE;
			}
		}
	}

	if (!braceClosed) {
		return XERR_CORRUPTED_FILE;
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
					return XERR_CORRUPTED_FILE;
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
				return XERR_CORRUPTED_FILE;
			}
		}
	}

	if (!bracketClosed) {
		return XERR_CORRUPTED_FILE;
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
					return XERR_CORRUPTED_FILE;
				}
				escaping = 0;
			} else {
				// unexpected character
				Array_Term(&buffer);
				return XERR_CORRUPTED_FILE;
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
					return XERR_CORRUPTED_FILE;
				}
			} else {
				// unexpected character
				Array_Term(&buffer);
				return XERR_CORRUPTED_FILE;
			}
		}
	}

	if (!quoteClosed) {
		// Quote not closed
		Array_Term(&buffer);
		return XERR_CORRUPTED_FILE;
	}

	c = 0;
	if (Array_Append(&buffer, &c) == NULL) {
		// out of memory, or max size reached
		Array_Term(&buffer);
		return XERR_CORRUPTED_FILE;
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
	return VSON_InitStringNoCopy(vson, Array_TermNoFree(&buffer));
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
				result = XERR_CORRUPTED_FILE;
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

XErr VSON_InitParseFile(VSON* vson, const char* path) {
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
				result = XERR_CORRUPTED_FILE;
				break;
			}
		}
	}
	
	fclose(f);
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
			for (long i = 0; i < index && arrV; i++) {
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

void VSON_Term(VSON* vson) {
	// TODO
}