#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>

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

// Convenience macros
#define ASSERT_TRUE(condition, err) \
	do {                  \
		if (!(condition)) \
			return (err); \
	} while (0)

#define ASSERT_TRUE_CLEANUP(condition, cleanup, err) \
	do {                    \
		if (!(condition)) { \
			{cleanup;}      \
			return (err);   \
		}                   \
	} while (0)

#define ASSERT_FALSE(condition, err) \
	do {                  \
		if ((condition))  \
			return (err); \
	} while (0)

#define ASSERT_FALSE_CLEANUP(condition, cleanup, err) \
	do {                   \
		if ((condition)) { \
			{cleanup;}     \
			return (err);  \
		}                  \
	} while (0)

#define REFLECT_ERROR(fcall)     \
	do {                         \
		XErr __result = (fcall); \
		if (__result < 0)        \
			return __result;     \
	} while (0)

#define REFLECT_ERROR_CLEANUP(fcall, cleanup) \
	do {                         \
		XErr __result = (fcall); \
		if (__result < 0) {      \
			{cleanup;}           \
			return __result;     \
		}                        \
	} while (0)

#endif
