#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>

#define PROPAGATE_ERROR(fcall)   \
	do {                         \
		XErr __result = (fcall); \
		if (__result < 0)        \
			return __result;     \
	} while (0)

#define PROPAGATE_ERROR_AFTER(fcall, cleanup_code) \
	do {                                           \
		XErr __result = (fcall);                   \
		if (__result < 0) {                        \
			{cleanup_code}                         \
			return __result;                       \
		}                                          \
	} while (0)

// Functions are not allowed to return positive values
#define XOK            (0)
#define XERR_QUIT      (-1)
#define XERR_NOT_FOUND (-2)
#define XERR_CORRUPTED (-3)
#define XERR_SDL       (-4)
#define XERR_MEMORY    (-5)
#define XERR_LIMIT     (-6)

typedef int32_t XErr;

#endif
