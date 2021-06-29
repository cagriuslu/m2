#include "Log.h"
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

static void LogHeader(LogLevel level, const char* file, int line) {
	time_t now = time(NULL) - 1577833200; // Epoch is 2020-01-01T00:00:00
	fprintf(stderr, "[%09llu:", now);

	const char* levelStr = " :";
	switch (level) {
		case LogLevelTrace:
			levelStr = "T:";
			break;
		case LogLevelDebug:
			levelStr = "D:";
			break;
		case LogLevelInfo:
			levelStr = "I:";
			break;
		case LogLevelWarn:
			levelStr = "W:";
			break;
		case LogLevelError:
			levelStr = "E:";
			break;
		case LogLevelFatal:
			levelStr = "F:";
			break;
	}
	fprintf(stderr, levelStr);

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
	const char* nodirFile = strrchr(file, '\\');
#else
	const char* nodirFile = strrchr(file, '/');
#endif
	const char* searchString = nodirFile == NULL ? file : nodirFile;
	
	char shortFile[6] = { 0 };
	int shortFileLen = 0;
	for (size_t i = 0; i < strlen(searchString); i++) {
		char c = searchString[i];
		if (isupper(c) || isdigit(c)) {
			shortFile[shortFileLen++] = searchString[i];
			if (5 == shortFileLen) {
				break;
			}
		}
	}
	const char* placeholders = "";
	switch (shortFileLen) {
		case 0:
			placeholders = "00000";
			break;
		case 1:
			placeholders = "0000";
			break;
		case 2:
			placeholders = "000";
			break;
		case 3:
			placeholders = "00";
			break;
		case 4:
			placeholders = "0";
			break;
	}
	fprintf(stderr, "%s%s:", placeholders, shortFile);
	
	fprintf(stderr, "%05d] ", line);
}

static void LogNewLine() {
	fprintf(stderr, "\n");
}

void Log(LogLevel level, const char* file, int line, const char* message) {
	LogHeader(level, file, line);
	if (message) {
		fprintf(stderr, "%s", message);
	}
	LogNewLine();
}

void LogTyp_Int32(LogLevel level, const char* file, int line, const char* message, int32_t var) {
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": %d}", message, var);
	LogNewLine();
}

void LogTyp_Float32(LogLevel level, const char* file, int line, const char* message, float var) {
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": %.4f}", message, var);
	LogNewLine();
}

void LogTyp_Vec2F(LogLevel level, const char* file, int line, const char* message, Vec2F var) {
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": {\"x\":%f, \"y\":%f}}", message, var.x, var.y);
	LogNewLine();
}

void LogTyp_Vec2I(LogLevel level, const char* file, int line, const char* message, Vec2I var) {
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": {\"x\":%d, \"y\":%d}}", message, var.x, var.y);
	LogNewLine();
}

void LogTyp_SDL_Rect(LogLevel level, const char* file, int line, const char* message, SDL_Rect var) {
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": {\"x\":%d, \"y\":%d, \"w\":%d, \"h\":%d}}", message, var.x, var.y, var.w, var.h);
	LogNewLine();
}

void LogTyp_ArrayInt32(LogLevel level, const char* file, int line, const char* message, Array* var) {
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
