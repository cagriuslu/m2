#include "TileLookup.h"
#include <string.h>

#define STREQ(str1, str2) (strcmp(str1, str2) == 0)

TileDef TileLookup(const char* tileName) {
	if (STREQ(tileName, "Ground")) {
		return (TileDef) { {0, 0}, { 0.0, 0.0 }, { 0.0, 0.0 } };
	} else if (STREQ(tileName, "WallHorizontal")) {
		return (TileDef) { {1, 0}, { 1.0, 1.0 }, { 0.0, 0.0 } };
	} else if (STREQ(tileName, "WallVertical")) {
		return (TileDef) { {0, 1}, { 1.0, 1.0 }, { 0.0, 0.0 } };
	} else {
		return (TileDef) { {0, 0}, { 1.0, 1.0 }, { 0.0, 0.0 } };
	}
}
