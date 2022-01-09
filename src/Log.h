#ifndef LOG_H
#define LOG_H

#include "Pool.h"
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

#define LOG_TRC(msg) Log(LogLevelTrace, __FILE__, __LINE__, (msg))
#define LOG_DBG(msg) Log(LogLevelDebug, __FILE__, __LINE__, (msg))
#define LOG_INF(msg) Log(LogLevelInfo,  __FILE__, __LINE__, (msg))
#define LOG_WRN(msg) Log(LogLevelWarn,  __FILE__, __LINE__, (msg))
#define LOG_ERR(msg) Log(LogLevelError, __FILE__, __LINE__, (msg))
#define LOG_FTL(msg) Log(LogLevelFatal, __FILE__, __LINE__, (msg))
void Log(LogLevel level, const char* file, int line, const char* message);

#define LOGX_TRC(x) LogX(LogLevelTrace, __FILE__, __LINE__, (x))
#define LOGX_DBG(x) LogX(LogLevelDebug, __FILE__, __LINE__, (x))
#define LOGX_INF(x) LogX(LogLevelInfo,  __FILE__, __LINE__, (x))
#define LOGX_WRN(x) LogX(LogLevelWarn,  __FILE__, __LINE__, (x))
#define LOGX_ERR(x) LogX(LogLevelError, __FILE__, __LINE__, (x))
#define LOGX_FTL(x) LogX(LogLevelFatal, __FILE__, __LINE__, (x))
XErr LogX(LogLevel level, const char* file, int line, XErr x);

#define LOGXV_TRC(x, typ, var) LogX_##typ(LogLevelTrace, __FILE__, __LINE__, (x), (var))
#define LOGXV_DBG(x, typ, var) LogX_##typ(LogLevelDebug, __FILE__, __LINE__, (x), (var))
#define LOGXV_INF(x, typ, var) LogX_##typ(LogLevelInfo,  __FILE__, __LINE__, (x), (var))
#define LOGXV_WRN(x, typ, var) LogX_##typ(LogLevelWarn,  __FILE__, __LINE__, (x), (var))
#define LOGXV_ERR(x, typ, var) LogX_##typ(LogLevelError, __FILE__, __LINE__, (x), (var))
#define LOGXV_FTL(x, typ, var) LogX_##typ(LogLevelFatal, __FILE__, __LINE__, (x), (var))
XErr LogX_ID(LogLevel level, const char* file, int line, XErr x, ID var);
XErr LogX_Int32(LogLevel level, const char* file, int line, XErr x, int32_t var);
XErr LogX_Float32(LogLevel level, const char* file, int line, XErr x, float var);
XErr LogX_String(LogLevel level, const char* file, int line, XErr x, const char* var);

#define LOG2XV_TRC(x1, typ1, var1, x2, typ2, var2) LogX_##typ1##_##typ2(LogLevelTrace, __FILE__, __LINE__, (x1), (var1), (x2), (var2))
#define LOG2XV_DBG(x1, typ1, var1, x2, typ2, var2) LogX_##typ1##_##typ2(LogLevelDebug, __FILE__, __LINE__, (x1), (var1), (x2), (var2))
#define LOG2XV_INF(x1, typ1, var1, x2, typ2, var2) LogX_##typ1##_##typ2(LogLevelInfo,  __FILE__, __LINE__, (x1), (var1), (x2), (var2))
#define LOG2XV_WRN(x1, typ1, var1, x2, typ2, var2) LogX_##typ1##_##typ2(LogLevelWarn,  __FILE__, __LINE__, (x1), (var1), (x2), (var2))
#define LOG2XV_ERR(x1, typ1, var1, x2, typ2, var2) LogX_##typ1##_##typ2(LogLevelError, __FILE__, __LINE__, (x1), (var1), (x2), (var2))
#define LOG2XV_FTL(x1, typ1, var1, x2, typ2, var2) LogX_##typ1##_##typ2(LogLevelFatal, __FILE__, __LINE__, (x1), (var1), (x2), (var2))
void LogX_ID_ID(LogLevel level, const char* file, int line, XErr x1, ID var1, XErr x2, ID var2);

#define LOG3XV_TRC(x1, typ1, var1, x2, typ2, var2, x3, typ3, var3) LogX_##typ1##_##typ2##_##typ3(LogLevelTrace, __FILE__, __LINE__, (x1), (var1), (x2), (var2), (x3), (var3))
#define LOG3XV_DBG(x1, typ1, var1, x2, typ2, var2, x3, typ3, var3) LogX_##typ1##_##typ2##_##typ3(LogLevelDebug, __FILE__, __LINE__, (x1), (var1), (x2), (var2), (x3), (var3))
#define LOG3XV_INF(x1, typ1, var1, x2, typ2, var2, x3, typ3, var3) LogX_##typ1##_##typ2##_##typ3(LogLevelInfo,  __FILE__, __LINE__, (x1), (var1), (x2), (var2), (x3), (var3))
#define LOG3XV_WRN(x1, typ1, var1, x2, typ2, var2, x3, typ3, var3) LogX_##typ1##_##typ2##_##typ3(LogLevelWarn,  __FILE__, __LINE__, (x1), (var1), (x2), (var2), (x3), (var3))
#define LOG3XV_ERR(x1, typ1, var1, x2, typ2, var2, x3, typ3, var3) LogX_##typ1##_##typ2##_##typ3(LogLevelError, __FILE__, __LINE__, (x1), (var1), (x2), (var2), (x3), (var3))
#define LOG3XV_FTL(x1, typ1, var1, x2, typ2, var2, x3, typ3, var3) LogX_##typ1##_##typ2##_##typ3(LogLevelFatal, __FILE__, __LINE__, (x1), (var1), (x2), (var2), (x3), (var3))
void LogX_ID_ID_Float32(LogLevel level, const char* file, int line, XErr x1, ID var1, XErr x2, ID var2, XErr x3, float var3);

// Convenience macros
#define LOGFN_TRC() LOGXV_TRC(XOK_FN, String, __FUNCTION__)
#define LOGFN_DBG() LOGXV_DBG(XOK_FN, String, __FUNCTION__)

#endif
