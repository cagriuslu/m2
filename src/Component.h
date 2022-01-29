#ifndef COMPONENT_H
#define COMPONENT_H

#include "Cfg.h"
#include "Vec2F.h"
#include "Box2D.h"
#include "Pool.h"
#include <SDL.h>
#include <stdint.h>

typedef struct _Component {
	ID objId;
} Component;
int Component_Init(Component* component, ID objectId);
void Component_Term(Component* component);

typedef struct _ComponentMonitor {
	Component super;
	void (*prePhysics)(struct _ComponentMonitor*);
	void (*postPhysics)(struct _ComponentMonitor*);
	void (*preGraphics)(struct _ComponentMonitor*);
	void (*postGraphics)(struct _ComponentMonitor*);
} ComponentMonitor;
int ComponentMonitor_Init(ComponentMonitor* evListener, ID objectId);
void ComponentMonitor_Term(ComponentMonitor* evListener);

typedef struct _ComponentPhysique {
	Component super;
	Box2DBody* body;
	void (*onCollision)(struct _ComponentPhysique*, struct _ComponentPhysique*);
} ComponentPhysique;
int ComponentPhysique_Init(ComponentPhysique* phy, ID objectId);
void ComponentPhysique_Term(ComponentPhysique* phy);
void ComponentPhysique_ContactCB(Box2DContact* contact);

typedef struct _ComponentGraphic {
	Component super;
	SDL_Texture *texture;
	SDL_Rect textureRect;
	Vec2F center_px;
	float angle;
	void (*draw)(struct _ComponentGraphic*);
	bool motionBlurEnabled;
	Vec2I prevObjGfxOriginWRTScreenCenter_px;
	float prevDrawAngle;
} ComponentGraphic;
int ComponentGraphic_Init(ComponentGraphic* gfx, ID objectId);
void ComponentGraphic_Term(ComponentGraphic* gfx);
void ComponentGraphic_DefaultDraw(ComponentGraphic* gfx);
void ComponentGraphic_DefaultDrawHealthBar(ComponentGraphic* gfx, float healthRatio);
Vec2I ComponentGraphic_GraphicsOriginWRTScreenCenter_px(Vec2F objPosition, Vec2F objGfxCenterPx);
int ComponentGraphic_YComparatorCB(ID gfxIdA, ID gfxIdB);

typedef struct _ComponentLight {
	Component super;
	float radius_m;
	void (*draw)(struct _ComponentLight*);
} ComponentLight;
XErr ComponentLight_Init(ComponentLight* lig, ID objectId);
void ComponentLight_Term(ComponentLight* lig);
void ComponentLight_DefaultDraw(ComponentLight* lig);

typedef struct _ComponentDefense {
	Component super;
	/// Non-zero group ID, if object belongs to a group
	uint16_t groupId;
	/// Index in group, if object belongs to a group
	uint16_t groupIdx;
	float maxHp;
	float hp;
	void (*onHit)(struct _ComponentDefense*);
	void (*onDeath)(struct _ComponentDefense*);
} ComponentDefense;
int ComponentDefense_Init(ComponentDefense* def, ID objId);
void ComponentDefense_Term(ComponentDefense* def);

typedef struct _ComponentOffense {
	Component super;
	ID originator;
	union {
		ProjectileState projectile;
		MeleeState melee;
		ExplosiveState explosive;
	} state;
} ComponentOffense;
int ComponentOffense_Init(ComponentOffense* def, ID objId);
void ComponentOffense_Term(ComponentOffense* def);

#endif
