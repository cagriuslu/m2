#ifndef CFG_H
#define CFG_H

#include "TinySet.hh"
#include "m2/Automaton.hh"
#include "List.hh"
#include "Vec2F-old.hh"
#include "Def.hh"
#include <SDL.h>

//#define CFG_TILE_SIZE (48)
//#define CFG_TILE_SIZE_F (48.0f)
//#define CFG_TEXTURE_FILE "resources/48.png"
//#define CFG_TEXTURE_MASK_FILE "resources/48-Mask.png"

////////////////////////////////////////////////////////////////////////
////////////////////////////////// UI //////////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef enum {
	CFG_UI_BUTTONTYP_INVALID = 0,
	CFG_UI_BUTTONTYP_QUIT,
	CFG_UI_BUTTONTYP_N
} CfgUIButtonType;
typedef enum {
	CFG_UI_DYNAMIC_TEXT_TYPE_INVALID = 0,
	CFG_UI_DYNAMIC_TEXT_TYPE_HP
} CfgUIDynamicTextType;
typedef enum {
	CFG_UI_DYNAMIC_IMAGE_TYPE_INVALID = 0,
} CfgUIDynamicImageType;
typedef enum {
	CFG_UI_ELEMENT_TYPE_INVALID = 0,
	CFG_UI_ELEMENT_TYP_UI,
	CFG_UI_ELEMENT_TYP_STATIC_TEXT,
	CFG_UI_ELEMENT_TYP_STATIC_TEXT_BUTTON,
	CFG_UI_ELEMENT_TYP_STATIC_IMAGE,
	CFG_UI_ELEMENT_TYP_STATIC_IMAGE_BUTTON,
	CFG_UI_ELEMENT_TYP_DYNAMIC_TEXT,
	CFG_UI_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON,
	CFG_UI_ELEMENT_TYP_DYNAMIC_IMAGE,
	CFG_UI_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON,
} CfgUIElementType;
typedef struct _CfgUIElement {
	unsigned x, y, w, h; // unitless
	unsigned borderWidth_px;
	SDL_Color backgroundColor;

	CfgUIElementType type;

	// Exists for UI
	const struct _CfgUI* child;
	// Exists for STATIC_TEXT, STATIC_TEXT_BUTTON
	const char* text;
	// Exists for STATIC_TEXT_BUTTON, STATIC_IMAGE_BUTTON, DYNAMIC_TEXT_BUTTON, DYNAMIC_IMAGE_BUTTON
	CfgUIButtonType buttonType;
	SDL_Scancode keyboardShortcut;
	// Exists for STATIC_IMAGE, STATIC_IMAGE_BUTTON
	const SDL_Rect texture;
	// Exists for DYNAMIC_TEXT, DYNAMIC_TEXT_BUTTON
	CfgUIDynamicTextType textType;
	// Exists for DYNAMIC_IMAGE, DYNAMIC_IMAGE_BUTTON
	CfgUIDynamicImageType imageType;

	const struct _CfgUIElement* next;
} CfgUIElement;
typedef struct _CfgUI {
	unsigned w, h; // unitless
	unsigned borderWidth_px;
	SDL_Color backgroundColor;
	const CfgUIElement* firstElement;
} CfgUI;
DECLARE_SIBLING_LIST_LENGTH_CALCULATOR(CfgUIElement);
typedef struct _UIElementState {
	SDL_Rect rect;
	const CfgUIElement* cfg;

	// Exists for UI
	struct _UIState* child;
	// Exists for STATIC_TEXT, STATIC_TEXT_BUTTON, DYNAMIC_TEXT, DYNAMIC_TEXT_BUTTON
	SDL_Texture* textTexture;
	// Exists for STATIC_TEXT_BUTTON, STATIC_IMAGE_BUTTON, DYNAMIC_TEXT_BUTTON, DYNAMIC_IMAGE_BUTTON
	bool depressed;
	// Exists for DYNAMIC_IMAGE, DYNAMIC_IMAGE_BUTTON
	const SDL_Rect texture;

	struct _UIElementState* next;
} UIElementState;
typedef struct _UIState {
	const CfgUI *cfg;
	SDL_Rect rect;
	UIElementState* firstElement;
} UIState;

////////////////////////////////////////////////////////////////////////
/////////////////////////////// SPRITES ////////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef enum {
	CFG_COLLIDER_TYPE_INVALID = 0,
	CFG_COLLIDER_TYPE_RECTANGLE,
	CFG_COLLIDER_TYPE_CIRCLE
} CfgColliderType;
typedef struct {
	Vec2F center_px;
	Vec2F center_m;
	CfgColliderType type;
	union {
		struct {
			Vec2F dims_px;
			Vec2F dims_m;
		} rect;
		struct {
			float radius_px;
			float radius_m;
		} circ;
	} params;
} CfgCollider;

typedef uint32_t CfgSpriteIndex;

typedef struct {
	CfgSpriteIndex index;
	SDL_Rect textureRect;
	Vec2F objCenter_px;
	Vec2F objCenter_m;
	CfgCollider collider;
} CfgSprite;

typedef struct {
	CfgSpriteIndex backgroundSpriteIndex;
	CfgSpriteIndex foregroundSpriteIndex;
} CfgTile;

////////////////////////////////////////////////////////////////////////
//////////////////////////////// LEVEL /////////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef struct {
	const CfgTile *tiles;
	int w, h;
} CfgLevel;

#endif
