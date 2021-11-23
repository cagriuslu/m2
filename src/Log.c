#include "Log.h"
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

static void LogHeader(LogLevel level, const char* file, int line) {
	// Get time
	time_t now = time(NULL);

	// Convert log level into char
	char levelChar = ' ';
	switch (level) {
		case LogLevelTrace:
			levelChar = 'T';
			break;
		case LogLevelDebug:
			levelChar = 'D';
			break;
		case LogLevelInfo:
			levelChar = 'I';
			break;
		case LogLevelWarn:
			levelChar = 'W';
			break;
		case LogLevelError:
			levelChar = 'E';
			break;
		case LogLevelFatal:
			levelChar = 'F';
			break;
		default:
			break;
	}

	// Get file name
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
	const char* fileNameWithoutDirs = strrchr(file, '\\');
#else
	const char* fileNameWithoutDirs = strrchr(file, '/');
#endif
	const char* fileName = (fileNameWithoutDirs == NULL) ? file : fileNameWithoutDirs;
	
	// Simplify file name
	char fileNameCapitals[6] = { 0 };
	int fileNameCapitalsLen = 0;
	for (size_t i = 0; i < strlen(fileName) && fileNameCapitalsLen < 5; i++) {
		char c = fileName[i];
		if (isupper(c) || isdigit(c)) {
			fileNameCapitals[fileNameCapitalsLen++] = c;
		}
	}
	const char* fileNameCapitalsPadding = "";
	switch (fileNameCapitalsLen) {
		case 0:
			fileNameCapitalsPadding = "-----";
			break;
		case 1:
			fileNameCapitalsPadding = "----";
			break;
		case 2:
			fileNameCapitalsPadding = "---";
			break;
		case 3:
			fileNameCapitalsPadding = "--";
			break;
		case 4:
			fileNameCapitalsPadding = "-";
			break;
		default:
			break;
	}
	
	fprintf(stderr, "[%010llu:%c:%s%s:%05d] ", (unsigned long long)now, levelChar, fileNameCapitalsPadding, fileNameCapitals, line);
}

static void LogNewLine() {
	fprintf(stderr, "\n");
}

void _Log(LogLevel level, const char* file, int line, const char* message) {
	LogHeader(level, file, line);
	if (message) {
		fprintf(stderr, "%s", message);
	}
	LogNewLine();
}

void _LogObj_Int32(LogLevel level, const char* file, int line, const char* message, int32_t var) {
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": %d}", message, var);
	LogNewLine();
}

void _LogObj_String(LogLevel level, const char* file, int line, const char* message, const char* var) {
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": \"%s\"}", message, var);
	LogNewLine();
}

void _LogObj_Float32(LogLevel level, const char* file, int line, const char* message, float var) {
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": %f}", message, var);
	LogNewLine();
}

void _LogObj_Vec2F(LogLevel level, const char* file, int line, const char* message, Vec2F var) {
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": {\"x\":%f, \"y\":%f}}", message, var.x, var.y);
	LogNewLine();
}

void _LogObj_Vec2I(LogLevel level, const char* file, int line, const char* message, Vec2I var) {
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": {\"x\":%d, \"y\":%d}}", message, var.x, var.y);
	LogNewLine();
}

void _LogObj_SdlRect(LogLevel level, const char* file, int line, const char* message, SDL_Rect var) {
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": {\"x\":%d, \"y\":%d, \"w\":%d, \"h\":%d}}", message, var.x, var.y, var.w, var.h);
	LogNewLine();
}

void _LogObj_ArrayOfInt32s(LogLevel level, const char* file, int line, const char* message, Array* var) {
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": [", message);
	for (size_t i = 0; i < var->length; i++) {
		fprintf(stderr, "%d", *(int32_t*)Array_Get(var, i));
		if (i + 1 < var->length) {
			fprintf(stderr, ", ");
		}
	}
	fprintf(stderr, "]}");
	LogNewLine();
}
