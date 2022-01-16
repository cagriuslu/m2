#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <stddef.h>

////////////////////////////////////////////////////////////////////////
////////////////////////////// Definitions /////////////////////////////
////////////////////////////////////////////////////////////////////////

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef uint64_t ID;

////////////////////////////////////////////////////////////////////////
///////////////////////////////// XERR /////////////////////////////////
////////////////////////////////////////////////////////////////////////

#define XOK_PROJECTILE_DMG          (  8)
#define XOK_PROJECTILE_DEATH        (  7)
#define XOK_HP                      (  6)
#define XOK_ID                      (  5)
#define XOK_BUTTON                  (  4)
#define XOK_FPS                     (  3)
#define XOK_LOG_LEVEL               (  2)
#define XOK_FN                      (  1)
#define XOK                         (  0)
#define XERR_FILE_NOT_FOUND         ( -1)
#define XERR_PATH_NOT_FOUND         ( -2)
#define XERR_FILE_CORRUPTED         ( -3)
#define XERR_SDL_ERROR              ( -4)
#define XERR_OUT_OF_MEMORY          ( -5)
#define XERR_LIMIT_EXCEEDED         ( -6)
#define XERR_ERRNO                  ( -7)
#define XERR_FILE_INACCESSIBLE      ( -8)
#define XERR_INVALID_CFG_OBJTYP     ( -9)
#define XERR_QUIT                   (-10)
#define XERR_INVALID_CFG_DYNTXTTYP  (-11)
#define XERR_INVALID_CFG_DYNIMGTYP  (-12)
#define XERR_IMPLEMENTATION         (-13)
#define XERR_TINYOBJ_LIMIT_EXCEEDED (-14)
#define XERR_OUT_OF_BOUNDS          (-15)
typedef int32_t XErr;

const char* XErr_ToString(XErr e);

// XASSERT policy: In game engine code, try not to crash, but return error instead
// You can crash in game engine code as well, if the error is definitely implementation error
// In game code, assert and crash if the cause is implementation error.
#ifdef DEBUG
#define XASSERT(cond) do { if (!(cond)) { LOG_FTL("Abort"); abort(); } } while (0);
#else
#define XASSERT(cond)
#endif
#define XERR_ASSERT(condition, err) \
	do {                  \
		if (!(condition)) \
			return (err); \
	} while (0)
#define XERR_ASSERT_CLEANUP(condition, cleanup, err) \
	do {                    \
		if (!(condition)) { \
			{cleanup;}      \
			return (err);   \
		}                   \
	} while (0)
#define XERR_REFLECT(fcall)     \
	do {                         \
		XErr __result = (fcall); \
		if (__result < 0)        \
			return __result;     \
	} while (0)
#define XERR_REFLECT_CLEANUP(fcall, cleanup) \
	do {                         \
		XErr __result = (fcall); \
		if (__result < 0) {      \
			{cleanup;}           \
			return __result;     \
		}                        \
	} while (0)

////////////////////////////////////////////////////////////////////////
///////////////////////////////// LOG //////////////////////////////////
////////////////////////////////////////////////////////////////////////

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

// Helper functions
void LogStackTrace();

////////////////////////////////////////////////////////////////////////
///////////////////////// Platform Abstraction /////////////////////////
////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#define STRDUP _strdup
#else
#define STRDUP strdup
#endif

////////////////////////////////////////////////////////////////////////
////////////////////////// Convenience Macros //////////////////////////
////////////////////////////////////////////////////////////////////////

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LERP(min,max,t) ((min) + (t) * ((max) - (min)))

#define DECLARE_SIBLING_LIST_LENGTH_CALCULATOR(typeName) \
	size_t SiblingListLength_##typeName(const typeName *ptr)
#define DEFINE_SIBLING_LIST_LENGTH_CALCULATOR(typeName) \
	DECLARE_SIBLING_LIST_LENGTH_CALCULATOR(typeName) { size_t len; for (len = 0; ptr; len++, ptr = ptr->next); return len; }

#endif
