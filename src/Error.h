#ifndef ERROR_H
#define ERROR_H

#define PROPAGATE_ERROR(fcall)  \
	do {                        \
		int __result = (fcall); \
		if (__result)           \
			return __result;    \
	} while (0)

#define PROPAGATE_ERROR_AFTER(fcall, cleanup_code) \
	do {                                           \
		int __result = (fcall);                    \
		if (__result) {                            \
			{cleanup_code}                         \
			return __result;                       \
		}                                          \
	} while (0)

#endif
