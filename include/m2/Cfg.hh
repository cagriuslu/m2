#ifndef CFG_H
#define CFG_H

#include "m2/FSM.h"
#include <m2/Vec2F.h>
#include "Def.hh"
#include <SDL.h>

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
struct UIElementState {
	SDL_Rect rect;
	const CfgUIElement* cfg;

	// Exists for UI
	struct _UIState* child;
	// Exists for STATIC_TEXT, STATIC_TEXT_BUTTON, DYNAMIC_TEXT, DYNAMIC_TEXT_BUTTON
	SDL_Texture* textTexture;
	// Exists for STATIC_TEXT_BUTTON, STATIC_IMAGE_BUTTON, DYNAMIC_TEXT_BUTTON, DYNAMIC_IMAGE_BUTTON
	bool depressed;
	// Exists for DYNAMIC_IMAGE, DYNAMIC_IMAGE_BUTTON
	SDL_Rect texture;

	struct UIElementState* next;
};
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
	m2::vec2f center_px;
	m2::vec2f center_m;
	CfgColliderType type;
	union {
		struct {
			m2::vec2f dims_px;
			m2::vec2f dims_m;
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
	m2::vec2f objCenter_px;
	m2::vec2f objCenter_m;
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
