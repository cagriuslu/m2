#ifndef LOG_H
#define LOG_H

#include "Array.h"
#include "Vec2I.h"
#include "Vec2F.h"
#include <SDL.h>
#include <stddef.h>
#include <stdint.h>

typedef enum _LogLevel {
	LogLevelTrace = 0,
	LogLevelDebug,
	LogLevelInfo,
	LogLevelWarn,
	LogLevelError,
	LogLevelFatal,
} LogLevel;

extern LogLevel gCurrentLogLevel;

#define LOG_TRC(msg) _Log(LogLevelTrace, __FILE__, __LINE__, msg)
#define LOG_DBG(msg) _Log(LogLevelDebug, __FILE__, __LINE__, msg)
#define LOG_INF(msg) _Log(LogLevelInfo,  __FILE__, __LINE__, msg)
#define LOG_WRN(msg) _Log(LogLevelWarn,  __FILE__, __LINE__, msg)
#define LOG_ERR(msg) _Log(LogLevelError, __FILE__, __LINE__, msg)
#define LOG_FTL(msg) _Log(LogLevelFatal, __FILE__, __LINE__, msg)

// Fixed variable names for LOGOBJ_ family of functions
#define LOGOBJ_FN "Fn"
#define LOGOBJ_FPS "FPS"
#define LOGOBJ_MENU_SELECTION "MenuSelection"
#define LOGOBJ_FILE_PATH "FilePath"
#define LOGOBJ_LOG_LEVEL "LogLevel"

#define LOGOBJ_TRC(msg, typ, var) _LogObj_##typ(LogLevelTrace, __FILE__, __LINE__, msg, var)
#define LOGOBJ_DBG(msg, typ, var) _LogObj_##typ(LogLevelDebug, __FILE__, __LINE__, msg, var)
#define LOGOBJ_INF(msg, typ, var) _LogObj_##typ(LogLevelInfo,  __FILE__, __LINE__, msg, var)
#define LOGOBJ_WRN(msg, typ, var) _LogObj_##typ(LogLevelWarn,  __FILE__, __LINE__, msg, var)
#define LOGOBJ_ERR(msg, typ, var) _LogObj_##typ(LogLevelError, __FILE__, __LINE__, msg, var)
#define LOGOBJ_FTL(msg, typ, var) _LogObj_##typ(LogLevelFatal, __FILE__, __LINE__, msg, var)

void _Log(LogLevel level, const char* file, int line, const char* message);
void _LogObj_Int32(LogLevel level, const char* file, int line, const char* message, int32_t var);
void _LogObj_Float32(LogLevel level, const char* file, int line, const char* message, float var);
void _LogObj_String(LogLevel level, const char* file, int line, const char* message, const char* var);
void _LogObj_Vec2F(LogLevel level, const char* file, int line, const char* message, Vec2F var);
void _LogObj_Vec2I(LogLevel level, const char* file, int line, const char* message, Vec2I var);
void _LogObj_SdlRect(LogLevel level, const char* file, int line, const char* message, SDL_Rect var);
void _LogObj_ArrayOfInt32s(LogLevel level, const char* file, int line, const char* message, Array* var);

// Convenience macros
#define LOGFN_TRC() LOGOBJ_TRC(LOGOBJ_FN, String, __FUNCTION__)
#define LOGFN_DBG() LOGOBJ_DBG(LOGOBJ_FN, String, __FUNCTION__)

#endif
