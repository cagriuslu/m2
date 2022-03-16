#define _CRT_SECURE_NO_WARNINGS
#include "m2/String.h"
#include "m2/Def.h"
#include "m2/String.h"

void String_Split_OutItemTerm(void* item) {
	char* ptr = *((char**)item);
	free(ptr);
}

M2Err String_Split(const char* string, char delimiter, Array* out) {
	M2Err result = Array_Init(out, sizeof(char*), 4, 65536, String_Split_OutItemTerm);
	if (result) {
		return result;
	}

	while (string) {
		char* delimiterPosition = strchr(string, delimiter);
		if (delimiterPosition) {
			const size_t buflen = delimiterPosition - string + 1;
			char* stringPiece = calloc(buflen, sizeof(char));
			if (!stringPiece) {
				return M2ERR_OUT_OF_MEMORY;
			}
			strncpy(stringPiece, string, buflen - 1);
			if (!Array_Append(out, &stringPiece)) {
				return M2ERR_LIMIT_EXCEEDED;
			}

			string = delimiterPosition + 1;
		} else {
			char* stringPiece = STRDUP(string);
			if (!stringPiece) {
				return M2ERR_OUT_OF_MEMORY;
			}
			strcpy(stringPiece, string);
			if (!Array_Append(out, &stringPiece)) {
				return M2ERR_LIMIT_EXCEEDED;
			}

			string = NULL;
		}
	}

	return 0;
}
