#include "m2/Def.hh"
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

LogLevel gCurrentLogLevel = LogLevelDebug;

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

#ifndef _WIN32
#include <execinfo.h>
#endif

void Log_StackTrace() {
#ifdef _WIN32
	// Not supported yet
#else
	void* callstack[128];
	int frames = backtrace(callstack, 128);
	char** strs = backtrace_symbols(callstack, frames);
	for (int i = 0; i < frames; ++i) {
		fprintf(stderr, "%s\n", strs[i]);
	}
	free(strs);
#endif
}

static void LogFooter(LogLevel level) {
	if (LogLevelWarn <= level) {
		Log_StackTrace();
	}
}

static void LogNewLine() {
	fprintf(stderr, "\n");
}

void Log(LogLevel level, const char* file, int line, const char* message) {
	if (level < gCurrentLogLevel) {
		return;
	}
	LogHeader(level, file, line);
	if (message) {
		fprintf(stderr, "%s", message);
	}
	LogNewLine();
	LogFooter(level);
}

M2Err Log_M2(LogLevel level, const char* file, int line, M2Err x) {
	if (level < gCurrentLogLevel) {
		return x;
	}
	LogHeader(level, file, line);
	fprintf(stderr, "{\"X\": \"%s\"}", M2Err_ToString(x));
	LogNewLine();
	LogFooter(level);
	return x;
}

M2Err Log_M2V_ID(LogLevel level, const char* file, int line, M2Err x, ID var) {
	if (level < gCurrentLogLevel) {
		return x;
	}
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": %016llX}", M2Err_ToString(x), var);
	LogNewLine();
	LogFooter(level);
	return x;
}

M2Err Log_M2V_Int32(LogLevel level, const char* file, int line, M2Err x, int32_t var) {
	if (level < gCurrentLogLevel) {
		return x;
	}
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": %d}", M2Err_ToString(x), var);
	LogNewLine();
	LogFooter(level);
	return x;
}

M2Err Log_M2V_Float32(LogLevel level, const char* file, int line, M2Err x, float var) {
	if (level < gCurrentLogLevel) {
		return x;
	}
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": %.3f}", M2Err_ToString(x), var);
	LogNewLine();
	LogFooter(level);
	return x;
}

M2Err Log_M2V_CString(LogLevel level, const char* file, int line, M2Err x, const char* var) {
	if (level < gCurrentLogLevel) {
		return x;
	}
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": \"%s\"}", M2Err_ToString(x), var);
	LogNewLine();
	LogFooter(level);
	return x;
}

M2Err Log_M2V_vec2f(LogLevel level, const char* file, int line, M2Err x, m2::vec2f var) {
	if (level < gCurrentLogLevel) {
		return x;
	}
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": {\"x\": %.3f, \"y\": %.3f}}", M2Err_ToString(x), var.x, var.y);
	LogNewLine();
	LogFooter(level);
	return x;
}

void Log_M2V_ID_ID(LogLevel level, const char* file, int line, M2Err x1, ID var1, M2Err x2, ID var2) {
	if (level < gCurrentLogLevel) {
		return;
	}
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": 0x%016llX, \"%s\": 0x%016llX}", M2Err_ToString(x1), var1, M2Err_ToString(x2), var2);
	LogNewLine();
	LogFooter(level);
}

void Log_M2V_CString_CString(LogLevel level, const char* file, int line, M2Err x1, const char* var1, M2Err x2, const char* var2) {
	if (level < gCurrentLogLevel) {
		return;
	}
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": \"%s\", \"%s\": \"%s\"}", M2Err_ToString(x1), var1, M2Err_ToString(x2), var2);
	LogNewLine();
	LogFooter(level);
}

void Log_M2V_ID_ID_Float32(LogLevel level, const char* file, int line, M2Err x1, ID var1, M2Err x2, ID var2, M2Err x3, float var3) {
	if (level < gCurrentLogLevel) {
		return;
	}
	LogHeader(level, file, line);
	fprintf(stderr, "{\"%s\": 0x%016llX, \"%s\": 0x%016llX, \"%s\": %.3f}", M2Err_ToString(x1), var1, M2Err_ToString(x2), var2,
			M2Err_ToString(x3), var3);
	LogNewLine();
	LogFooter(level);
}
