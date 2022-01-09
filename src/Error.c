#include "Error.h"

static const char* XErr_PositiveStrings[] = {
	"OK",       // XOK
	"Fn",       // XOK_FN
	"LgLvl",    // XOK_LOG_LEVEL
	"FPS",      // XOK_FPS
	"Bttn",     // XOK_BUTTON
	"ID",       // XOK_ID
	"HP",       // XOK_HP
	"ProjDth",  // XOK_PROJECTILE_DEATH
	"ProjDmg",  // XOK_PROJECTILE_DMG
};
static const char* XErr_NegativeStrings[] = {
	"OK",                         // XOK
	"FileNotFound",               // XERR_FILE_NOT_FOUND
	"PathNotFound",               // XERR_PATH_NOT_FOUND
	"FileCorrupted",              // XERR_FILE_CORRUPTED
	"SdlError",                   // XERR_SDL_ERROR
	"OutOfMemory",                // XERR_OUT_OF_MEMORY
	"LimitExceeded",              // XERR_LIMIT_EXCEEDED
	"Errno",                      // XERR_ERRNO
	"FileInaccessible",           // XERR_FILE_INACCESSIBLE
	"InvalidCfgObjectType",       // XERR_INVALID_CFG_OBJTYP
	"Quit",                       // XERR_QUIT
	"InvalidCfgDynamicTextType",  // XERR_INVALID_CFG_DYNTXTTYP
	"InvalidCfgDynamicImageType", // XERR_INVALID_CFG_DYNIMGTYP
	"ImplementationError"         // XERR_IMPLEMENTATION
};

const char* XErr_ToString(XErr e) {
	return (0 < e) ? XErr_PositiveStrings[e] : XErr_NegativeStrings[-e];
}
