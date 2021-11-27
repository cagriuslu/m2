#include "TileSet.h"
#include "VSON.h"
#include <string.h>

XErr TileSet_InitFromFile(TileSet* ts, const char* fpath) {
	memset(ts, 0, sizeof(TileSet));
	VSON vson;
	XErr result = VSON_InitParseFile(&vson, fpath);
	if (result) {
		return result;
	}
	if (vson.type != VSON_VALUE_TYPE_OBJECT) {
		return XERR_CORRUPTED;
	}
	
}
