#include "Error.h"

static const char* XErr_PositiveStrings[] = {
	"OK",       // XOK
	"Fn",       // XOK_FN
	"LogLevel", // XOK_LOG_LEVEL
	"FPS",      // XOK_FPS
	"Button"    // XOK_BUTTON
};
static const char* XErr_NegativeStrings[] = {
	"OK",                 // XOK
	"FileNotFound",       // XERR_FILE_NOT_FOUND
	"PathNotFound",       // XERR_PATH_NOT_FOUND
	"FileCorrupted",      // XERR_FILE_CORRUPTED
	"SdlError",           // XERR_SDL_ERROR
	"OutOfMemory",        // XERR_OUT_OF_MEMORY
	"LimitExceeded",      // XERR_LIMIT_EXCEEDED
	"Errno",              // XERR_ERRNO
	"FileInaccessible",   // XERR_FILE_INACCESSIBLE
	"ObjectTypeNotFound", // XERR_OBJTYP_NOT_FOUND
	"Quit",               // XERR_QUIT
};

const char* XErr_ToString(XErr e) {
	return (0 < e) ? XErr_PositiveStrings[e] : XErr_NegativeStrings[-e];
}
