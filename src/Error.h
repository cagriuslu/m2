#ifndef ERROR_H
#define ERROR_H

#define PROPAGATE_ERROR(fcall)  \
	do {                        \
		int __result = (fcall); \
		if (__result < 0)       \
			return __result;    \
	} while (0)

#define PROPAGATE_ERROR_AFTER(fcall, cleanup_code) \
	do {                                           \
		int __result = (fcall);                    \
		if (__result < 0) {                        \
			{cleanup_code}                         \
			return __result;                       \
		}                                          \
	} while (0)

#define X_QUIT (-1)

#endif
