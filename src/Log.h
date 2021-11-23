#ifndef LOG_H
#define LOG_H

#include "Array.h"
#include "Vec2I.h"
#include "Vec2F.h"
#include <SDL.h>
#include <stddef.h>
#include <stdint.h>

#define LOG_TRC(msg) Log(LogLevelTrace, __FILE__, __LINE__, msg)
#define LOG_DBG(msg) Log(LogLevelDebug, __FILE__, __LINE__, msg)
#define LOG_INF(msg) Log(LogLevelInfo,  __FILE__, __LINE__, msg)
#define LOG_WRN(msg) Log(LogLevelWarn,  __FILE__, __LINE__, msg)
#define LOG_ERR(msg) Log(LogLevelError, __FILE__, __LINE__, msg)
#define LOG_FTL(msg) Log(LogLevelFatal, __FILE__, __LINE__, msg)

#define LOGOBJ_TRC(msg, typ, var) LogObj_##typ(LogLevelTrace, __FILE__, __LINE__, msg, var)
#define LOGOBJ_DBG(msg, typ, var) LogObj_##typ(LogLevelDebug, __FILE__, __LINE__, msg, var)
#define LOGOBJ_INF(msg, typ, var) LogObj_##typ(LogLevelInfo,  __FILE__, __LINE__, msg, var)
#define LOGOBJ_WRN(msg, typ, var) LogObj_##typ(LogLevelWarn,  __FILE__, __LINE__, msg, var)
#define LOGOBJ_ERR(msg, typ, var) LogObj_##typ(LogLevelError, __FILE__, __LINE__, msg, var)
#define LOGOBJ_FTL(msg, typ, var) LogObj_##typ(LogLevelFatal, __FILE__, __LINE__, msg, var)

typedef enum _LogLevel {
	LogLevelTrace = 0,
	LogLevelDebug,
	LogLevelInfo,
	LogLevelWarn,
	LogLevelError,
	LogLevelFatal,
} LogLevel;

void Log(LogLevel level, const char* file, int line, const char* message);

void LogObj_Int32(LogLevel level, const char* file, int line, const char* message, int32_t var);
void LogObj_Float32(LogLevel level, const char* file, int line, const char* message, float var);
void LogObj_String(LogLevel level, const char* file, int line, const char* message, const char* var);
void LogObj_Vec2F(LogLevel level, const char* file, int line, const char* message, Vec2F var);
void LogObj_Vec2I(LogLevel level, const char* file, int line, const char* message, Vec2I var);
void LogObj_SDL_Rect(LogLevel level, const char* file, int line, const char* message, SDL_Rect var);
void LogObj_ArrayOfInt32s(LogLevel level, const char* file, int line, const char* message, Array* var);

// Fixed variable names for LOGOBJ_ family of functions
#define LOGVAR_FPS "FPS"
#define LOGVAR_MENU_SELECTION "MenuSelection"
#define LOGVAR_FILE_PATH "FilePath"

#endif
