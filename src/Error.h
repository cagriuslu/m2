#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>

// Functions are not allowed to return positive values
#define XOK            (0)
#define XERR_QUIT      (-1)
#define XERR_NOT_FOUND (-2)
#define XERR_CORRUPTED (-3)
#define XERR_SDL       (-4)
#define XERR_MEMORY    (-5)
#define XERR_LIMIT     (-6)
#define XERR_ARG       (-7)

typedef int32_t XErr;

// Convenience macros
#define ASSERT_TRUE(condition, err) \
	do {                  \
		if (!(condition)) \
			return (err); \
	} while (0)

#define ASSERT_TRUE_CLEANUP(condition, err, cleanup) \
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

#define ASSERT_FALSE_CLEANUP(condition, err, cleanup) \
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
