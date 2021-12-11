#ifndef LOG_H
#define LOG_H

#include "Error.h"
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

#define LOG_TRC(msg) Log(LogLevelTrace, __FILE__, __LINE__, msg)
#define LOG_DBG(msg) Log(LogLevelDebug, __FILE__, __LINE__, msg)
#define LOG_INF(msg) Log(LogLevelInfo,  __FILE__, __LINE__, msg)
#define LOG_WRN(msg) Log(LogLevelWarn,  __FILE__, __LINE__, msg)
#define LOG_ERR(msg) Log(LogLevelError, __FILE__, __LINE__, msg)
#define LOG_FTL(msg) Log(LogLevelFatal, __FILE__, __LINE__, msg)

#define LOGX_TRC(x) LogX(LogLevelTrace, __FILE__, __LINE__, x)
#define LOGX_DBG(x) LogX(LogLevelDebug, __FILE__, __LINE__, x)
#define LOGX_INF(x) LogX(LogLevelInfo,  __FILE__, __LINE__, x)
#define LOGX_WRN(x) LogX(LogLevelWarn,  __FILE__, __LINE__, x)
#define LOGX_ERR(x) LogX(LogLevelError, __FILE__, __LINE__, x)
#define LOGX_FTL(x) LogX(LogLevelFatal, __FILE__, __LINE__, x)

#define LOGXV_TRC(x, typ, var) LogX_##typ(LogLevelTrace, __FILE__, __LINE__, x, var)
#define LOGXV_DBG(x, typ, var) LogX_##typ(LogLevelDebug, __FILE__, __LINE__, x, var)
#define LOGXV_INF(x, typ, var) LogX_##typ(LogLevelInfo,  __FILE__, __LINE__, x, var)
#define LOGXV_WRN(x, typ, var) LogX_##typ(LogLevelWarn,  __FILE__, __LINE__, x, var)
#define LOGXV_ERR(x, typ, var) LogX_##typ(LogLevelError, __FILE__, __LINE__, x, var)
#define LOGXV_FTL(x, typ, var) LogX_##typ(LogLevelFatal, __FILE__, __LINE__, x, var)

void Log(LogLevel level, const char* file, int line, const char* message);
void LogX(LogLevel level, const char* file, int line, XErr x);
void LogX_Int32(LogLevel level, const char* file, int line, XErr x, int32_t var);
void LogX_String(LogLevel level, const char* file, int line, XErr x, const char* var);

// Convenience macros
#define LOGFN_TRC() LOGXV_TRC(XOK_FN, String, __FUNCTION__)
#define LOGFN_DBG() LOGXV_DBG(XOK_FN, String, __FUNCTION__)

#endif
