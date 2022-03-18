#ifndef DEFS_H
#define DEFS_H

#include "Vec2F-old.hh"
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

#define M2_2PI  6.283185307179586f // 2pi
#define M2_PI   3.141592653589793f // pi
#define M2_PI2  1.570796326794897f // pi/2
#define M2_PI4  0.785398163397448f // pi/4
#define M2_3PI4 2.356194490192345f // 3pi/4

typedef uint64_t ID;

////////////////////////////////////////////////////////////////////////
///////////////////////////////// XERR /////////////////////////////////
////////////////////////////////////////////////////////////////////////

#define M2_PROJECTILE_DMG            (  8)
#define M2_PROJECTILE_DEATH          (  7)
#define M2_HP                        (  6)
#define M2_ID                        (  5)
#define M2_BUTTON                    (  4)
#define M2_FPS                       (  3)
#define M2_LOG_LEVEL                 (  2)
#define M2_FN                        (  1)
#define M2OK                         (  0)
#define M2ERR_FILE_NOT_FOUND         ( -1)
#define M2ERR_PATH_NOT_FOUND         ( -2)
#define M2ERR_FILE_CORRUPTED         ( -3)
#define M2ERR_SDL_ERROR              ( -4)
#define M2ERR_OUT_OF_MEMORY          ( -5)
#define M2ERR_LIMIT_EXCEEDED         ( -6)
#define M2ERR_ERRNO                  ( -7)
#define M2ERR_FILE_INACCESSIBLE      ( -8)
#define M2ERR_INVALID_CFG_OBJTYP     ( -9)
#define M2ERR_QUIT                   (-10)
#define M2ERR_INVALID_CFG_DYNTXTTYP  (-11)
#define M2ERR_INVALID_CFG_DYNIMGTYP  (-12)
#define M2ERR_IMPLEMENTATION         (-13)
#define M2ERR_TINYOBJ_LIMIT_EXCEEDED (-14)
#define M2ERR_OUT_OF_BOUNDS          (-15)
#define M2ERR_PROXY_CHANGED          (-16)
typedef int32_t M2Err;

const char* M2Err_ToString(M2Err e);

// M2ASSERT policy: In game engine code, try not to crash, but return error instead
// You can crash in game engine code as well, if the error is definitely implementation error
// In game code, assert and crash if the cause is implementation error.
#ifdef DEBUG
#define M2ASSERT(cond) do { if (!(cond)) { LOG_FATAL("Abort"); abort(); } } while (0);
#else
#define M2ASSERT(cond)
#endif
#define M2ERR_ASSERT(condition, err) \
	do {                  \
		if (!(condition)) \
			return (err); \
	} while (0)
#define M2ERR_ASSERT_CLEANUP(condition, cleanup, err) \
	do {                    \
		if (!(condition)) { \
			{cleanup;}      \
			return (err);   \
		}                   \
	} while (0)
#define M2ERR_REFLECT(fcall)      \
	do {                          \
		M2Err __result = (fcall); \
		if (__result < 0)         \
			return __result;      \
	} while (0)
#define M2ERR_REFLECT_CLEANUP(fcall, cleanup) \
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

#define LOG_TRACE(msg)   Log(LogLevelTrace, __FILE__, __LINE__, (msg))
#define LOG_DEBUG(msg)   Log(LogLevelDebug, __FILE__, __LINE__, (msg))
#define LOG_INFO(msg)    Log(LogLevelInfo,  __FILE__, __LINE__, (msg))
#define LOG_WARNING(msg) Log(LogLevelWarn,  __FILE__, __LINE__, (msg))
#define LOG_ERROR(msg)   Log(LogLevelError, __FILE__, __LINE__, (msg))
#define LOG_FATAL(msg)   Log(LogLevelFatal, __FILE__, __LINE__, (msg))
void Log(LogLevel level, const char* file, int line, const char* message);

#define LOG_TRACE_M2(x)   Log_M2(LogLevelTrace, __FILE__, __LINE__, (x))
#define LOG_DEBUG_M2(x)   Log_M2(LogLevelDebug, __FILE__, __LINE__, (x))
#define LOG_INFO_M2(x)    Log_M2(LogLevelInfo,  __FILE__, __LINE__, (x))
#define LOG_WARNING_M2(x) Log_M2(LogLevelWarn,  __FILE__, __LINE__, (x))
#define LOG_ERROR_M2(x)   Log_M2(LogLevelError, __FILE__, __LINE__, (x))
#define LOG_FATAL_M2(x)   Log_M2(LogLevelFatal, __FILE__, __LINE__, (x))
M2Err Log_M2(LogLevel level, const char* file, int line, M2Err x);

#define LOG_TRACE_M2V(x, typ, var)   Log_M2V_##typ(LogLevelTrace, __FILE__, __LINE__, (x), (var))
#define LOG_DEBUG_M2V(x, typ, var)   Log_M2V_##typ(LogLevelDebug, __FILE__, __LINE__, (x), (var))
#define LOG_INFO_M2V(x, typ, var)    Log_M2V_##typ(LogLevelInfo,  __FILE__, __LINE__, (x), (var))
#define LOG_WARNING_M2V(x, typ, var) Log_M2V_##typ(LogLevelWarn,  __FILE__, __LINE__, (x), (var))
#define LOG_ERROR_M2V(x, typ, var)   Log_M2V_##typ(LogLevelError, __FILE__, __LINE__, (x), (var))
#define LOG_FATAL_M2V(x, typ, var)   Log_M2V_##typ(LogLevelFatal, __FILE__, __LINE__, (x), (var))
M2Err Log_M2V_ID(LogLevel level, const char* file, int line, M2Err x, ID var);
M2Err Log_M2V_Int32(LogLevel level, const char* file, int line, M2Err x, int32_t var);
M2Err Log_M2V_Float32(LogLevel level, const char* file, int line, M2Err x, float var);
M2Err Log_M2V_String(LogLevel level, const char* file, int line, M2Err x, const char* var);
M2Err Log_M2V_Vec2F(LogLevel level, const char* file, int line, M2Err x, Vec2F var);

#define LOG_TRACE_M2VV(x1, typ1, var1, x2, typ2, var2)   Log_M2V_##typ1##_##typ2(LogLevelTrace, __FILE__, __LINE__, (x1), (var1), (x2), (var2))
#define LOG_DEBUG_M2VV(x1, typ1, var1, x2, typ2, var2)   Log_M2V_##typ1##_##typ2(LogLevelDebug, __FILE__, __LINE__, (x1), (var1), (x2), (var2))
#define LOG_INFO_M2VV(x1, typ1, var1, x2, typ2, var2)    Log_M2V_##typ1##_##typ2(LogLevelInfo,  __FILE__, __LINE__, (x1), (var1), (x2), (var2))
#define LOG_WARNING_M2VV(x1, typ1, var1, x2, typ2, var2) Log_M2V_##typ1##_##typ2(LogLevelWarn,  __FILE__, __LINE__, (x1), (var1), (x2), (var2))
#define LOG_ERROR_M2VV(x1, typ1, var1, x2, typ2, var2)   Log_M2V_##typ1##_##typ2(LogLevelError, __FILE__, __LINE__, (x1), (var1), (x2), (var2))
#define LOG_FATAL_M2VV(x1, typ1, var1, x2, typ2, var2)   Log_M2V_##typ1##_##typ2(LogLevelFatal, __FILE__, __LINE__, (x1), (var1), (x2), (var2))
void Log_M2V_ID_ID(LogLevel level, const char* file, int line, M2Err x1, ID var1, M2Err x2, ID var2);

#define LOG_TRACE_M2VVV(x1, typ1, var1, x2, typ2, var2, x3, typ3, var3)   Log_M2V_##typ1##_##typ2##_##typ3(LogLevelTrace, __FILE__, __LINE__, (x1), (var1), (x2), (var2), (x3), (var3))
#define LOG_DEBUG_M2VVV(x1, typ1, var1, x2, typ2, var2, x3, typ3, var3)   Log_M2V_##typ1##_##typ2##_##typ3(LogLevelDebug, __FILE__, __LINE__, (x1), (var1), (x2), (var2), (x3), (var3))
#define LOG_INFO_M2VVV(x1, typ1, var1, x2, typ2, var2, x3, typ3, var3)    Log_M2V_##typ1##_##typ2##_##typ3(LogLevelInfo,  __FILE__, __LINE__, (x1), (var1), (x2), (var2), (x3), (var3))
#define LOG_WARNING_M2VVV(x1, typ1, var1, x2, typ2, var2, x3, typ3, var3) Log_M2V_##typ1##_##typ2##_##typ3(LogLevelWarn,  __FILE__, __LINE__, (x1), (var1), (x2), (var2), (x3), (var3))
#define LOG_ERROR_M2VVV(x1, typ1, var1, x2, typ2, var2, x3, typ3, var3)   Log_M2V_##typ1##_##typ2##_##typ3(LogLevelError, __FILE__, __LINE__, (x1), (var1), (x2), (var2), (x3), (var3))
#define LOG_FATAL_M2VVV(x1, typ1, var1, x2, typ2, var2, x3, typ3, var3)   Log_M2V_##typ1##_##typ2##_##typ3(LogLevelFatal, __FILE__, __LINE__, (x1), (var1), (x2), (var2), (x3), (var3))
void Log_M2V_ID_ID_Float32(LogLevel level, const char* file, int line, M2Err x1, ID var1, M2Err x2, ID var2, M2Err x3, float var3);

// Convenience macros
#define LOG_TRACE_FN() LOG_TRACE_M2V(M2_FN, String, __FUNCTION__)
#define LOG_DEBUG_FN() LOG_DEBUG_M2V(M2_FN, String, __FUNCTION__)

// Helper functions
void Log_StackTrace();

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
#define ACCURACY(value, accuracy) ((value) + ((value) * randf() * (1 - (accuracy))) - ((value) * (1 - (accuracy)) / 2.0f))
float NORMALIZE_2PI(float angle);
/// Generate a random number between 0.0f and 1.0f
float randf();

#define DECLARE_SIBLING_LIST_LENGTH_CALCULATOR(typeName) \
	size_t SiblingListLength_##typeName(const typeName *ptr)
#define DEFINE_SIBLING_LIST_LENGTH_CALCULATOR(typeName) \
	DECLARE_SIBLING_LIST_LENGTH_CALCULATOR(typeName) { size_t len; for (len = 0; ptr; len++, ptr = ptr->next); return len; }

#endif
