#ifndef LOG_H
#define LOG_H

#include "Vec2I.h"
#include "Vec2F.h"
#include <SDL.h>
#include <stddef.h>
#include <stdint.h>

#include "Array.h"

#define LOG_TRC(msg) Log(LogLevelTrace, __FILE__, __LINE__, msg)
#define LOG_DBG(msg) Log(LogLevelDebug, __FILE__, __LINE__, msg)
#define LOG_INF(msg) Log(LogLevelInfo,  __FILE__, __LINE__, msg)
#define LOG_WRN(msg) Log(LogLevelWarn,  __FILE__, __LINE__, msg)
#define LOG_ERR(msg) Log(LogLevelError, __FILE__, __LINE__, msg)
#define LOG_FTL(msg) Log(LogLevelFatal, __FILE__, __LINE__, msg)

#define LOGTYP_TRC(msg, typ, var) LogTyp_##typ(LogLevelTrace, __FILE__, __LINE__, msg, var)
#define LOGTYP_DBG(msg, typ, var) LogTyp_##typ(LogLevelDebug, __FILE__, __LINE__, msg, var)
#define LOGTYP_INF(msg, typ, var) LogTyp_##typ(LogLevelInfo,  __FILE__, __LINE__, msg, var)
#define LOGTYP_WRN(msg, typ, var) LogTyp_##typ(LogLevelWarn,  __FILE__, __LINE__, msg, var)
#define LOGTYP_ERR(msg, typ, var) LogTyp_##typ(LogLevelError, __FILE__, __LINE__, msg, var)
#define LOGTYP_FTL(msg, typ, var) LogTyp_##typ(LogLevelFatal, __FILE__, __LINE__, msg, var)

typedef enum _LogLevel {
	LogLevelTrace = 0,
	LogLevelDebug,
	LogLevelInfo,
	LogLevelWarn,
	LogLevelError,
	LogLevelFatal,
} LogLevel;

void Log(LogLevel level, const char* file, int line, const char* message);

void LogTyp_Int32(LogLevel level, const char* file, int line, const char* message, int32_t var);
void LogTyp_Float32(LogLevel level, const char* file, int line, const char* message, float var);
void LogTyp_String(LogLevel level, const char* file, int line, const char* message, const char* var);
void LogTyp_Vec2F(LogLevel level, const char* file, int line, const char* message, Vec2F var);
void LogTyp_Vec2I(LogLevel level, const char* file, int line, const char* message, Vec2I var);
void LogTyp_SDL_Rect(LogLevel level, const char* file, int line, const char* message, SDL_Rect var);
void LogTyp_ArrayOfInt32s(LogLevel level, const char* file, int line, const char* message, Array* var);

// Fixed variable names for LOGTYP_ family of functions
#define LOGVAR_FPS "FPS"
#define LOGVAR_MENU_SELECTION "MenuSelection"
#define LOGVAR_FILE_PATH "FilePath"

#endif
