#include "m2/Def.hh"

static const char* M2Err_PositiveStrings[] = {
	"OK",       // M2OK
	"Fn",       // M2_FN
	"LogLvl",   // M2_LOG_LEVEL
	"FPS",      // M2_FPS
	"Button",   // M2_BUTTON
	"ID",       // M2_ID
	"HP",       // M2_HP
	"ProjDth",  // M2_PROJECTILE_DEATH
	"ProjDmg",  // M2_PROJECTILE_DMG
};
static const char* M2Err_NegativeStrings[] = {
	"OK",                         // M2OK
	"FileNotFound",               // M2ERR_FILE_NOT_FOUND
	"PathNotFound",               // M2ERR_PATH_NOT_FOUND
	"FileCorrupted",              // M2ERR_FILE_CORRUPTED
	"SdlError",                   // M2ERR_SDL_ERROR
	"OutOfMemory",                // M2ERR_OUT_OF_MEMORY
	"LimitExceeded",              // M2ERR_LIMIT_EXCEEDED
	"Errno",                      // M2ERR_ERRNO
	"FileInaccessible",           // M2ERR_FILE_INACCESSIBLE
	"InvalidCfgObjectType",       // M2ERR_INVALID_CFG_OBJTYP
	"Quit",                       // M2ERR_QUIT
	"InvalidCfgDynamicTextType",  // M2ERR_INVALID_CFG_DYNTXTTYP
	"InvalidCfgDynamicImageType", // M2ERR_INVALID_CFG_DYNIMGTYP
	"ImplementationError",        // M2ERR_IMPLEMENTATION
	"TinyObjectLimitExceeded",    // M2ERR_TINYOBJ_LIMIT_EXCEEDED
	"OutOfBounds",                // M2ERR_OUT_OF_BOUNDS
};

const char* M2Err_ToString(M2Err e) {
	return (0 < e) ? M2Err_PositiveStrings[e] : M2Err_NegativeStrings[-e];
}
